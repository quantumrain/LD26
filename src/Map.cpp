#include "Pch.h"
#include "Common.h"
#include "Game.h"

void map::destroy() {
	delete [] data;
	size = ivec2();
	data = 0;
}

vec2 to_screen(ivec2 v) {
	return vec2((float)v.x, (float)-v.y);
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

void set_map(map* m, game_state* gs, int x, int y, uint8_t c) {
	bool clear_tile = c == ' ';

	if (c >= '0' && c <= '9') {
		int num = c - '0';

		if (num < MAX_WORMS) {
			worm* w = &gs->worms[num];

			gs->num_worms = Max(gs->num_worms, num + 1);

			if (w->num_blocks < MAX_WORM_BLOCKS) {
				worm_block* b = &w->blocks[w->num_blocks++];

				b->pos = ivec2(x, y);
				b->age = w->age++;
			}
		}

		clear_tile = true;
	}

	if (c >= 'A' && c <= 'Z') {
		int num = c - 'A';

		if (num < MAX_WORMS) {
			m->targets[num] = ivec2(x, y);
		}
	}

	if (clear_tile)
		m->data[y * m->size.x + x] = TILE_EMPTY;
}

bool load_map(map* m, game_state* gs, const char* path) {
	m->destroy();
	gs->reset();

	file_buf fb;

	if (!load_file(&fb, path)) {
		DebugLn("Failed to load map: %s", path);
		return false;
	}

	m->size = measure_map(fb.data, fb.data + fb.size);

	if (m->size.x == 0 || m->size.y == 0) {
		m->destroy();
		return false;
	}

	m->data = new uint8_t[m->size.x * m->size.y];

	if (!m->data) {
		m->destroy();
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
			set_map(m, gs, x, y, *p);
			x++;
			p++;
		}

		if (p < e && ((*p == '\r' && p[1] == '\n') || (*p == '\n' && p[1] == '\r')))
			p++;
	}

	float c0 = 0.45f;
	float c1 = 0.95f;

	colour cols[MAX_WORMS] = {
		colour(c1, c0, c0, 1),
		colour(c0, c1, c0, 1),
		colour(c0, c0, c1, 1),
		colour(c1, c1, c0, 1),
		colour(c0, c1, c1, 1),
		colour(c1, c0, c1, 1),
	};

	for(int i = 0; i < gs->num_worms; i++) {
		m->colours[i] = cols[i];
	}

	return true;
}

void render_map(map* m, game_state* gs, vec2 scale) {
	ivec2 size = m->size;

	for(int y = 0; y < size.y; y++) {
		for(int x = 0; x < size.x; x++) {
			vec2 p0(to_screen(ivec2(x, y)));
			vec2 p1(p0 + vec2(1.0f));

			colour c(0);

			switch(m->at(x, y)) {
				case TILE_EMPTY: c = colour(0.25f, 1.0f); break;
			}

			draw_rect(p0 * scale, p1 * scale, c);
		}
	}

	for(int i = 0; i < gs->num_worms; i++) {
		worm* w = gs->worms + i;

		// worm

		for(int j = 0; j < w->num_blocks; j++) {
			worm_block* b = w->blocks + j;

			vec2 p0(to_screen(b->pos));
			vec2 p1(p0 + vec2(1.0f));

			draw_rect(p0 * scale, p1 * scale, m->colours[i]);
		}

		// target

		vec2 p0(to_screen(m->targets[i]) + vec2(0.05f));
		vec2 p1(p0 + vec2(0.9f));

		draw_rect(p0 * scale, p1 * scale, m->colours[i]);
	}

	worm* w = gs->worms + gs->active_worm;
	worm_block* b = w->blocks + w->active_block;

	vec2 p0(to_screen(b->pos) + vec2(0.25f));
	vec2 p1(p0 + vec2(0.5f));

	draw_rect(p0 * scale, p1 * scale, colour(0.0f, 0.5f));
}