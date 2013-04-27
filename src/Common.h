#ifndef COMMON_H
#define COMMON_H

template<typename T> T Max(T a, T b) { return (b < a) ? a : b; }
template<typename T> T Min(T a, T b) { return (a < b) ? a : b; }
template<typename T> T Clamp(T v, T low, T high) { return (v < low) ? low : ((v > high) ? high : v); }
template<typename T> T Lerp(T a, T b, float t) { return a + (b - a) * t; }

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

inline vec2 operator+(const vec2& lhs, const vec2& rhs) { return vec2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline vec2 operator-(const vec2& lhs, const vec2& rhs) { return vec2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline vec2 operator*(const vec2& lhs, const vec2& rhs) { return vec2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline vec2 operator/(const vec2& lhs, const vec2& rhs) { return vec2(lhs.x / rhs.x, lhs.y / rhs.y); }

inline vec2 operator-(const vec2& rhs) { return vec2(-rhs.x, -rhs.y); }

inline vec2& operator+=(vec2& lhs, const vec2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline vec2& operator-=(vec2& lhs, const vec2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline vec2& operator*=(vec2& lhs, const vec2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline vec2& operator/=(vec2& lhs, const vec2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

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
inline vec2 RandBox(float magX, float magY) { return vec2(SignedFRand(magX), SignedFRand(magY)); }

int hash(const uint8_t* data, int size);

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
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0
};

extern int gKey;

// Debug

void DebugLn(const char* txt, ...);
void Panic(const char* msg);

// Gpu

extern int kWinWidth;
extern int kWinHeight;

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

	Texture2d* CreateTexture2d(int width, int height);
	void DestroyTexture2d(Texture2d* tex);
	void SetTexture(Texture2d* tex);

	void Init();
	void Clear(uint32_t col);
	void SetPsConst(int slot, vec4 v);
	void SetVsConst(int slot, vec4 v);
	void Draw(ShaderDecl* decl, VertexBuffer* vb, int count);

}

// Render

struct Vertex
{
	float x, y;
	float u, v;
	float r, g, b, a;
};

void set_camera(vec2 centre, float width);
void draw_rect(vec2 p0, vec2 p1, colour c);

// Sound

enum SoundId
{
	kSid_Select,
	kSid_Back,
	kSid_Jump,
	kSid_KittenJump,
	kSid_Land,
	kSid_Dead,
	kSid_CheckPoint,
	kSid_Kitten,
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

#endif // COMMON_H