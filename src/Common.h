#ifndef COMMON_H
#define COMMON_H

const float DT = 1.0f / 60.0f;
const float PI = 3.14159f;

template<typename T> T Max(T a, T b) { return (b < a) ? a : b; }
template<typename T> T Min(T a, T b) { return (a < b) ? a : b; }
template<typename T> T Clamp(T v, T low, T high) { return (v < low) ? low : ((v > high) ? high : v); }
template<typename T> T Lerp(T a, T b, float t) { return a + (b - a) * t; }

inline float Square(float f) { return f * f; }

template<typename T> void Swap(T& a, T& b)
{
	T t = b;
	b = a;
	a = t;
}

struct vec2
{
	float x, y;

	vec2(float xy = 0) : x(xy), y(xy) { }
	vec2(float x_, float y_) : x(x_), y(y_) { }
};

struct vec4
{
	float x, y, z, w;

	vec4(float xyzw = 0) : x(xyzw), y(xyzw), z(xyzw), w(xyzw) { }
	vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) { }
	vec4(vec2 xy, float z_, float w_) : x(xy.x), y(xy.y), z(z_), w(w_) { }
	vec4(vec2 xy, vec2 zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) { }
};

struct ivec2 {
	int x, y;

	ivec2(int xy = 0) : x(xy), y(xy) { }
	ivec2(int x_, int y_) : x(x_), y(y_) { }
};

inline vec2 operator+(const vec2& lhs, const vec2& rhs) { return vec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline vec2 operator-(const vec2& lhs, const vec2& rhs) { return vec2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline vec2 operator*(const vec2& lhs, const vec2& rhs) { return vec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline vec2 operator/(const vec2& lhs, const vec2& rhs) { return vec2(lhs.x / rhs.x, lhs.y / rhs.y); }

inline vec2 operator-(const vec2& rhs) { return vec2(-rhs.x, -rhs.y); }

inline vec2& operator+=(vec2& lhs, const vec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline vec2& operator-=(vec2& lhs, const vec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline vec2& operator*=(vec2& lhs, const vec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline vec2& operator/=(vec2& lhs, const vec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

inline vec4 operator+(const vec4& lhs, const vec4& rhs) { return vec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w); }
inline vec4 operator-(const vec4& lhs, const vec4& rhs) { return vec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w); }
inline vec4 operator*(const vec4& lhs, const vec4& rhs) { return vec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }
inline vec4 operator/(const vec4& lhs, const vec4& rhs) { return vec4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w); }

inline vec4 operator-(const vec4& rhs) { return vec4(-rhs.x, -rhs.y, -rhs.z, -rhs.w); }

inline vec4& operator+=(vec4& lhs, const vec4& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; lhs.z += rhs.z; lhs.w += rhs.w; return lhs; }
inline vec4& operator-=(vec4& lhs, const vec4& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; lhs.z -= rhs.z; lhs.w -= rhs.w; return lhs; }
inline vec4& operator*=(vec4& lhs, const vec4& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; lhs.z *= rhs.z; lhs.w *= rhs.w; return lhs; }
inline vec4& operator/=(vec4& lhs, const vec4& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; lhs.z /= rhs.z; lhs.w /= rhs.w; return lhs; }

inline ivec2 operator+(const ivec2& lhs, const ivec2& rhs) { return ivec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline ivec2 operator-(const ivec2& lhs, const ivec2& rhs) { return ivec2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline ivec2 operator*(const ivec2& lhs, const ivec2& rhs) { return ivec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline ivec2 operator/(const ivec2& lhs, const ivec2& rhs) { return ivec2(lhs.x / rhs.x, lhs.y / rhs.y); }

inline ivec2 operator-(const ivec2& rhs) { return ivec2(-rhs.x, -rhs.y); }

inline ivec2& operator+=(ivec2& lhs, const ivec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline ivec2& operator-=(ivec2& lhs, const ivec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline ivec2& operator*=(ivec2& lhs, const ivec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline ivec2& operator/=(ivec2& lhs, const ivec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

inline bool operator==(ivec2& lhs, ivec2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
inline bool operator!=(ivec2& lhs, ivec2& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y; }

inline vec2 to_vec2(ivec2 v) { return vec2((float)v.x, (float)v.y); }
inline vec2 Rotation(float a) { return vec2(cosf(a), sinf(a)); }

struct colour
{
	float r, g, b, a;

	colour(float rgba = 1) : r(rgba), g(rgba), b(rgba), a(rgba) { }
	colour(float rgb, float a) : r(rgb), g(rgb), b(rgb), a(a) { }
	colour(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) { }
};

inline colour operator+(const colour& lhs, const colour& rhs) { return colour(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a); }
inline colour operator-(const colour& lhs, const colour& rhs) { return colour(lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a); }
inline colour operator*(const colour& lhs, const colour& rhs) { return colour(lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b, lhs.a * rhs.a); }
inline colour operator/(const colour& lhs, const colour& rhs) { return colour(lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b, lhs.a / rhs.a); }

inline float FRand(float mag) { return ((float)rand() / (float)RAND_MAX) * mag; }
inline float SignedFRand(float mag) { return FRand(2.0f * mag) - mag; }
inline vec2 RandBox(float magX, float magY) { return vec2(FRand(magX), FRand(magY)); }
inline vec2 SignedRandBox(float magX, float magY) { return vec2(SignedFRand(magX), SignedFRand(magY)); }

int hash(const uint8_t* data, int size);
float gaussian(float n, float theta);

inline bool OverlapsRect(vec2 c0, vec2 s0, vec2 c1, vec2 s1)
{
	vec2 delta = c1 - c0;
	vec2 size = (s0 + s1) * 0.5f;
	
	return (fabsf(delta.x) < size.x) && (fabsf(delta.y) < size.y);
}

enum key_press {
	KEY_NONE,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_FIRE,
	KEY_ALT_FIRE,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,
	KEY_RESET,
	KEY_CHEAT,
	KEY_MODE,
	KEY_MAX
};

extern ivec2 g_WinSize;
extern int gKey;
extern ivec2 gMousePos;
extern int gMouseButtons;

// Debug

void DebugLn(const char* txt, ...);
void Panic(const char* msg);

// Gpu

namespace gpu
{

	struct VertexBuffer;
	struct ShaderDecl;
	struct Texture2d;

	VertexBuffer* CreateVertexBuffer(int elementSize, int elementCount);
	void DestroyVertexBuffer(VertexBuffer* vb);
	void* Map(VertexBuffer* vb);
	void Unmap(VertexBuffer* vb);

	ShaderDecl* CreateShaderDecl(const BYTE* vertexShader, int vertexShaderLength, const BYTE* pixelShader, int pixelShaderLength);
	void DestroyShaderDecl(ShaderDecl* decl);

	Texture2d* CreateTexture2d(int width, int height, uint8_t* initial_data);
	void DestroyTexture2d(Texture2d* tex);
	void SetSampler(int slot, bool tex_clamp, bool bilin);
	void SetTexture(int slot, Texture2d* tex);

	void Init();
	void Shutdown();

	void Clear(uint32_t col);
	void SetViewport(ivec2 pos, ivec2 size, vec2 depth);
	void SetRenderTarget(Texture2d* tex);
	void SetDefaultRenderTarget();
	void SetPsConst(int slot, vec4 v);
	void SetVsConst(int slot, vec4 v);
	void Draw(ShaderDecl* decl, VertexBuffer* vb, int count, bool alpha_blend, bool as_lines);

}

// Render

struct Vertex
{
	float x, y;
	float u, v;
	float r, g, b, a;
};

void set_camera(vec2 centre, float width);
vec2 to_game(vec2 screen);
void set_tint(colour tint);
void draw_rect(vec2 p0, vec2 p1, colour c);
void draw_rect(vec2 p0, vec2 p1, colour c0, colour c1, colour c2, colour c3);
void draw_quad(vec2 p0, vec2 p1, vec2 p2, vec2 p3, colour c0, colour c1, colour c2, colour c3);
void draw_font_rect(vec2 p0, vec2 p1, vec2 uv0, vec2 uv1, colour col);

enum font_flags {
	TEXT_LEFT = 1,
	TEXT_RIGHT = 2,
	TEXT_CENTRE = 3
};

void draw_char(vec2 pos, vec2 scale, int sprite, colour col);
float measure_char(int c);
float measure_string(const char* txt);
void draw_string(vec2 pos, vec2 scale, int flags, colour col, const char* txt, ...);

// Sound

enum SoundId
{
	kSid_Dit,
	kSid_Buzz,
	kSid_Switch,
	kSid_Win,
	kSid_Max
};

void SoundInit();
void SoundShutdown();
void SoundPlay(SoundId sid, float freq, float volume);

// File

struct file_buf {
	uint8_t* data;
	int size;

	file_buf() : data(), size() { }
	~file_buf() { destroy(); }

	void destroy() { delete [] data; data = 0; size = 0; }
};

bool load_file(file_buf* fb, const char* path);
gpu::Texture2d* load_texture(const char* path);

#endif // COMMON_H