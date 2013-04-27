// Copyright 2012 Stephen Cakebread

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

struct Vector2
{
	float x, y;

	Vector2(float xy = 0) : x(xy), y(xy) { }
	Vector2(float x_, float y_) : x(x_), y(y_) { }
};

inline Vector2 operator+(const Vector2& lhs, const Vector2& rhs) { return Vector2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline Vector2 operator-(const Vector2& lhs, const Vector2& rhs) { return Vector2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline Vector2 operator*(const Vector2& lhs, const Vector2& rhs) { return Vector2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline Vector2 operator/(const Vector2& lhs, const Vector2& rhs) { return Vector2(lhs.x / rhs.x, lhs.y / rhs.y); }

inline Vector2 operator-(const Vector2& rhs) { return Vector2(-rhs.x, -rhs.y); }

inline Vector2& operator+=(Vector2& lhs, const Vector2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline Vector2& operator-=(Vector2& lhs, const Vector2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline Vector2& operator*=(Vector2& lhs, const Vector2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline Vector2& operator/=(Vector2& lhs, const Vector2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

inline Vector2 Rotation(float a) { return Vector2(cosf(a), sinf(a)); }

struct Colour
{
	float r, g, b, a;

	Colour(float rgba = 1) : r(rgba), g(rgba), b(rgba), a(rgba) { }
	Colour(float rgb, float a) : r(rgb), g(rgb), b(rgb), a(a) { }
	Colour(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) { }
};

inline Colour operator+(const Colour& lhs, const Colour& rhs) { return Colour(lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a); }
inline Colour operator-(const Colour& lhs, const Colour& rhs) { return Colour(lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a); }
inline Colour operator*(const Colour& lhs, const Colour& rhs) { return Colour(lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b, lhs.a * rhs.a); }
inline Colour operator/(const Colour& lhs, const Colour& rhs) { return Colour(lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b, lhs.a / rhs.a); }

extern bool gHasFocus;
extern bool gKeyUp;
extern bool gKeyDown;
extern bool gKeyLeft;
extern bool gKeyRight;
extern bool gKeyFire;

// Main

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

	ShaderDecl* CreateShaderDecl(void* vertexShader, int vertexShaderLength, void* pixelShader, int pixelShaderLength);
	void DestroyShaderDecl(ShaderDecl* decl);

	Texture2d* LoadTexture2d(const char* path);
	void DestroyTexture2d(Texture2d* tex);
	void SetTexture(Texture2d* tex);

	void Init();
	void Clear(const Colour& colour);
	void Draw(ShaderDecl* decl, VertexBuffer* vb, int count);

}

// Render

struct Vertex
{
	Vector2 pos;
	Vector2 uv;
	Colour colour;
};

enum { kFlipX = 1, kFlipY = 2 };

void DrawRect(Vector2 p0, Vector2 p1, int sprite, int flags, Colour colour);
void DrawSprite(Vector2 pos, Vector2 scale, int sprite, int flags, Colour colour);

enum { kTextLeft = 1, kTextCentre = 2, kTextRight = 3 };

void DrawCharRect(Vector2 p0, Vector2 p1, int sprite, int flags, Colour colour);
void DrawChar(Vector2 pos, Vector2 scale, int sprite, int flags, Colour colour);
void DrawString(Vector2 pos, int flags, Colour colour, const char* txt, ...);

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

#endif // COMMON_H