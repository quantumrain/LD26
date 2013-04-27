#include "Pch.h"
#include "Common.h"
#include "quad_ps.h"
#include "quad_vs.h"

gpu::VertexBuffer* gRectVb;
gpu::ShaderDecl* gRectDecl;
gpu::Texture2d* gRectTex;

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

void RenderInit()
{
	gRectVb		= gpu::CreateVertexBuffer(sizeof(Vertex), kMaxRectVerts);
	gRectDecl	= gpu::CreateShaderDecl(g_quad_vs, sizeof(g_quad_vs), g_quad_ps, sizeof(g_quad_ps));
}

void RenderShutdown()
{
	gpu::DestroyVertexBuffer(gRectVb);
	gpu::DestroyShaderDecl(gRectDecl);
	gpu::DestroyTexture2d(gRectTex);
}

void RenderPreUpdate()
{
	gRectVertCount = 0;
}

void RenderGame()
{
	if (void* data = gpu::Map(gRectVb))
	{
		memcpy(data, gRectVerts, gRectVertCount * sizeof(Vertex));
		gpu::Unmap(gRectVb);
	}

	gpu::Clear(0x00000000);
	gpu::SetTexture(gRectTex);
	gpu::SetVsConst(0, gCam);
	gpu::Draw(gRectDecl, gRectVb, gRectVertCount);
}