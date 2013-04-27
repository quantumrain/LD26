#include "Pch.h"
#include "Common.h"
#include "quad_ps.h"
#include "quad_vs.h"
#include "reduce_ps.h"
#include "blur_x_ps.h"
#include "blur_y_ps.h"
#include "combine_ps.h"

gpu::VertexBuffer* gRectVb;
gpu::VertexBuffer* gFsQuadVb;
gpu::ShaderDecl* gRectDecl;

gpu::ShaderDecl* gReduceDecl;
gpu::ShaderDecl* gBlurXDecl;
gpu::ShaderDecl* gBlurYDecl;
gpu::ShaderDecl* gCombineDecl;

const int kMaxRectVerts = 64 * 1024;
Vertex gRectVerts[kMaxRectVerts];
int gRectVertCount;
vec4 gCam;

void set_camera(vec2 centre, float width) {
	float ratio = kWinHeight / (float)kWinWidth;
	gCam = vec4(-centre, 1.0f / vec2(width, -width * ratio));
}

void draw_rect(vec2 p0, vec2 p1, colour colour) {
	if ((gRectVertCount + 6) > kMaxRectVerts)
	{
		DebugLn("DrawRect overflow");
		return;
	}

	Vertex* v = &gRectVerts[gRectVertCount];

	gRectVertCount += 6;

	// t0

	v->x = p0.x;
	v->y = p0.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = colour.r;
	v->g = colour.g;
	v->b = colour.b;
	v->a = colour.a;
	v++;

	v->x = p0.x;
	v->y = p1.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = colour.r;
	v->g = colour.g;
	v->b = colour.b;
	v->a = colour.a;
	v++;

	v->x = p1.x;
	v->y = p0.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = colour.r;
	v->g = colour.g;
	v->b = colour.b;
	v->a = colour.a;
	v++;

	// t1

	v->x = p1.x;
	v->y = p0.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = colour.r;
	v->g = colour.g;
	v->b = colour.b;
	v->a = colour.a;
	v++;

	v->x = p0.x;
	v->y = p1.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = colour.r;
	v->g = colour.g;
	v->b = colour.b;
	v->a = colour.a;
	v++;

	v->x = p1.x;
	v->y = p1.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = colour.r;
	v->g = colour.g;
	v->b = colour.b;
	v->a = colour.a;
}

void draw_rect(vec2 p0, vec2 p1, colour c0, colour c1, colour c2, colour c3) {
	if ((gRectVertCount + 6) > kMaxRectVerts)
	{
		DebugLn("DrawRect overflow");
		return;
	}

	Vertex* v = &gRectVerts[gRectVertCount];

	gRectVertCount += 6;

	// t0

	v->x = p0.x;
	v->y = p0.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = c0.r;
	v->g = c0.g;
	v->b = c0.b;
	v->a = c0.a;
	v++;

	v->x = p0.x;
	v->y = p1.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = c2.r;
	v->g = c2.g;
	v->b = c2.b;
	v->a = c2.a;
	v++;

	v->x = p1.x;
	v->y = p0.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = c1.r;
	v->g = c1.g;
	v->b = c1.b;
	v->a = c1.a;
	v++;

	// t1

	v->x = p1.x;
	v->y = p0.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = c1.r;
	v->g = c1.g;
	v->b = c1.b;
	v->a = c1.a;
	v++;

	v->x = p0.x;
	v->y = p1.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = c2.r;
	v->g = c2.g;
	v->b = c2.b;
	v->a = c2.a;
	v++;

	v->x = p1.x;
	v->y = p1.y;
	v->u = 0.0f;
	v->v = 0.0f;
	v->r = c3.r;
	v->g = c3.g;
	v->b = c3.b;
	v->a = c3.a;
}

struct bloom_level {
	ivec2 size;
	gpu::Texture2d* reduce;
	gpu::Texture2d* blur_x;
	gpu::Texture2d* blur_y;

	bloom_level() : reduce(), blur_x(), blur_y() { }
	~bloom_level() { destroy(); }

	void destroy() {
		gpu::DestroyTexture2d(reduce);
		gpu::DestroyTexture2d(blur_x);
		gpu::DestroyTexture2d(blur_y);

		reduce = 0;
		blur_x = 0;
		blur_y = 0;
	}
};

const int MAX_BLOOM_LEVELS = 5;
bloom_level g_bloom_levels[MAX_BLOOM_LEVELS];
gpu::Texture2d* g_draw_target;

void RenderInit()
{
	gRectVb		= gpu::CreateVertexBuffer(sizeof(Vertex), kMaxRectVerts);
	gRectDecl	= gpu::CreateShaderDecl(g_quad_vs, sizeof(g_quad_vs), g_quad_ps, sizeof(g_quad_ps));

	gFsQuadVb = gpu::CreateVertexBuffer(sizeof(Vertex), 3);

	gReduceDecl = gpu::CreateShaderDecl(g_quad_vs, sizeof(g_quad_vs), g_reduce_ps, sizeof(g_reduce_ps));
	gBlurXDecl = gpu::CreateShaderDecl(g_quad_vs, sizeof(g_quad_vs), g_blur_x_ps, sizeof(g_blur_x_ps));
	gBlurYDecl = gpu::CreateShaderDecl(g_quad_vs, sizeof(g_quad_vs), g_blur_y_ps, sizeof(g_blur_y_ps));
	gCombineDecl = gpu::CreateShaderDecl(g_quad_vs, sizeof(g_quad_vs), g_combine_ps, sizeof(g_combine_ps));

	ivec2 size(kWinWidth, kWinHeight);

	g_draw_target = gpu::CreateTexture2d(size.x, size.y);

	for(int i = 0; i < MAX_BLOOM_LEVELS; i++) {
		bloom_level* bl = g_bloom_levels + i;

		size /= 2;

		bl->size = size;
		bl->reduce = gpu::CreateTexture2d(size.x, size.y);
		bl->blur_x = gpu::CreateTexture2d(size.x, size.y);
		bl->blur_y = gpu::CreateTexture2d(size.x, size.y);
	}
}

void RenderShutdown()
{
	gpu::DestroyVertexBuffer(gRectVb);
	gpu::DestroyShaderDecl(gRectDecl);
}

void RenderPreUpdate()
{
	gRectVertCount = 0;
}

void do_fullscreen_quad(gpu::ShaderDecl* decl, vec2 size)
{
	if (Vertex* v = (Vertex*)gpu::Map(gFsQuadVb)) {
		float ax = -(1.0f / size.x);
		float ay = (1.0f / size.y);

		v->x = -1.0f + ax;
		v->y = -1.0f + ay;
		v->u = 0.0f; // TODO: pixel centres...
		v->v = 1.0f;
		v->r = v->g = v->b = v->a = 1.0f;
		v++;

		v->x = -1.0f + ax;
		v->y = 3.0f + ay;
		v->u = 0.0f;
		v->v = -1.0f;
		v->r = v->g = v->b = v->a = 1.0f;
		v++;

		v->x = 3.0f + ax;
		v->y = -1.0f + ay;
		v->u = 2.0f;
		v->v = 1.0f;
		v->r = v->g = v->b = v->a = 1.0f;

		gpu::Unmap(gFsQuadVb);
	}

	gpu::SetVsConst(0, vec4(0, 0, 1.0f, 1.0f));
	gpu::Draw(decl, gFsQuadVb, 3, false);
}

void RenderGame()
{
	if (void* data = gpu::Map(gRectVb))
	{
		memcpy(data, gRectVerts, gRectVertCount * sizeof(Vertex));
		gpu::Unmap(gRectVb);
	}

	// draw
	gpu::SetRenderTarget(g_draw_target);
	gpu::SetViewport(ivec2(0, 0), ivec2(kWinWidth, kWinHeight), vec2(0.0f, 1.0f));
	gpu::Clear(0x00000000);
	gpu::SetVsConst(0, gCam);
	gpu::Draw(gRectDecl, gRectVb, gRectVertCount, true);

	for(int i = 0; i < MAX_BLOOM_LEVELS; i++) {
		bloom_level* bl = g_bloom_levels + i;

		// reduce
		gpu::SetRenderTarget(bl->reduce);
		gpu::SetViewport(ivec2(), bl->size, vec2(0.0f, 1.0f));
		gpu::SetTexture(0, (i == 0) ? g_draw_target : g_bloom_levels[i - 1].blur_y);
		gpu::SetSampler(0, true, true);
		gpu::SetPsConst(0, vec4(1.0f / to_vec2(bl->size), 0.0f, 0.0f));
		do_fullscreen_quad(gReduceDecl, to_vec2(bl->size));

		// blur x
		gpu::SetRenderTarget(bl->blur_x);
		gpu::SetViewport(ivec2(), bl->size, vec2(0.0f, 1.0f));
		gpu::SetTexture(0, bl->reduce);
		gpu::SetSampler(0, true, true);
		gpu::SetPsConst(0, vec4(1.0f / to_vec2(bl->size), 0.0f, 0.0f));
		do_fullscreen_quad(gBlurXDecl, to_vec2(bl->size));

		// blur y
		gpu::SetRenderTarget(bl->blur_y);
		gpu::SetViewport(ivec2(), bl->size, vec2(0.0f, 1.0f));
		gpu::SetTexture(0, bl->blur_x);
		gpu::SetSampler(0, true, true);
		gpu::SetPsConst(0, vec4(1.0f / to_vec2(bl->size), 0.0f, 0.0f));
		do_fullscreen_quad(gBlurYDecl, to_vec2(bl->size));
	}

	// combine
	gpu::SetDefaultRenderTarget();
	gpu::SetViewport(ivec2(0, 0), ivec2(kWinWidth, kWinHeight), vec2(0.0f, 1.0f));
	gpu::SetPsConst(0, vec4(500.0f) + vec4(FRand(1000.0f), FRand(1000.0f), FRand(1000.0f), FRand(1000.0f)));
	gpu::SetTexture(0, g_draw_target);
	gpu::SetSampler(0, true, false);
	for(int i = 0; i < MAX_BLOOM_LEVELS; i++) {
		gpu::SetTexture(1 + i, g_bloom_levels[i].blur_y);
		gpu::SetSampler(1 + i, true, true);
	}
	do_fullscreen_quad(gCombineDecl, to_vec2(ivec2(kWinWidth, kWinHeight)));
}