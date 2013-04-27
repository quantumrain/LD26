#include "Pch.h"
#include "Common.h"

int hash(const uint8_t* data, int size) {
	int h = 5381;

	while(size-- > 0)
		h = h * 33 + *data;

	return h;
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