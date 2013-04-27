#include "Pch.h"
#include "Common.h"
#include "Game.h"

void destroy_map(map* m) {
	delete [] m->data;
	m->data = 0;
}

ivec2 measure_map(const uint8_t* p, const uint8_t* e) {
	ivec2 size(0, 0);

	for(; p < e; p++) {
		if (*p != '#')
			break;

		int x = 0;

		while(p < e && *p != '\r' && *p != '\n') {
			x++;
			p++;
		}

		if (p < e && ((*p == '\r' && p[1] == '\n') || (*p == '\n' && p[1] == '\r')))
			p++;

		size.x = Max(size.x, x);
		size.y++;
	}

	return size;
}

void set_map(map* m, int x, int y, uint8_t c) {
	uint8_t* p = m->data + (y * m->size.x) + x;

	switch(c) {
		case '#': *p = TILE_WALL; break;
		case ' ': *p = TILE_EMPTY; break;
	}
}

bool load_map(map* m, const char* path) {
	destroy_map(m);

	file_buf fb;

	if (!load_file(&fb, path)) {
		DebugLn("Failed to load map: %s", path);
		return false;
	}

	m->size = measure_map(fb.data, fb.data + fb.size);

	if (m->size.x == 0 || m->size.y == 0) {
		destroy_map(m);
		return false;
	}

	m->data = new uint8_t[m->size.x * m->size.y];

	if (!m->data) {
		destroy_map(m);
		return false;
	}

	bool new_line = false;
	bool add_line = false;

	const uint8_t* p = fb.data;
	const uint8_t* e = fb.data + fb.size;

	for(int y = 0; p < e; p++, y++) {
		if (*p != '#')
			break;

		int x = 0;

		while(p < e && *p != '\r' && *p != '\n') {
			set_map(m, x, y, *p);
			x++;
			p++;
		}

		if (p < e && ((*p == '\r' && p[1] == '\n') || (*p == '\n' && p[1] == '\r')))
			p++;
	}

	return true;
}

void render_map(map* m, vec2 scale) {
	ivec2 size = m->size;

	for(int y = 0; y < size.y; y++) {
		for(int x = 0; x < size.x; x++) {
			vec2 p0((float)x, (float)-y);
			vec2 p1(p0 + vec2(1.0f));

			colour c(0);

			switch(m->at(x, y)) {
				case TILE_EMPTY: c = colour(0.25f, 1.0f); break;
			}

			draw_rect(p0 * scale, p1 * scale, c);
		}
	}
}