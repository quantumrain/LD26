#include "Pch.h"
#include "Common.h"

#define STBI_NO_STDIO
#include "stb_image.c"

extern HWND gMainWnd;

void DebugLn(const char* txt, ...)
{
	char buf[512];

	va_list ap;

	va_start(ap, txt);
	_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, txt, ap);
	va_end(ap);

	OutputDebugStringA(buf);
	OutputDebugStringA("\r\n");
}

void Panic(const char* msg)
{
	MessageBoxA(gMainWnd, msg, "LD26 - Gravity Worm", MB_ICONERROR | MB_OK);
	ExitProcess(0);
}

int hash(const uint8_t* data, int size) {
	int h = 5381;

	while(size-- > 0)
		h = h * 33 + *data;

	return h;
}

float gaussian(float n, float theta) {
	return ((1.0f / sqrtf(2.0f * 3.14159f * theta)) * expf(-(n * n) / (2.0f * theta * theta)));   
}

bool load_file(file_buf* fb, const char* path) {
	fb->destroy();

	HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (h == INVALID_HANDLE_VALUE)
		return false;

	DWORD size = GetFileSize(h, 0);

	if (size < (64 * 1024 * 1024)) {
		if ((fb->data = new uint8_t[size]) != 0) {
			fb->size = (int)size;
			DWORD bytes = 0;
			if (!ReadFile(h, fb->data, size, &bytes, 0) || (bytes != size)) {
				fb->destroy();
			}
		}
	}

	CloseHandle(h);

	return fb->data != 0;
}

gpu::Texture2d* load_texture(const char* path) {
	file_buf fb;

	if (!load_file(&fb, path))
		return 0;

	int width = 0, height = 0;
	uint8_t* data = stbi_load_from_memory(fb.data, fb.size, &width, &height, 0, 4);

	gpu::Texture2d* tex = gpu::CreateTexture2d(width, height, data);

	free(data);

	return tex;
}