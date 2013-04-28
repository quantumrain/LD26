#include "Pch.h"
#include "Common.h"
#include "Game.h"

#define s0 0.15f
#define s1 0.5f
#define s2 0.7f
#define s4 0.25f

#define c0 0.35f
#define c1 0.85f
#define c2 0.2f
#define c3 0.0f

int g_colour_mode;

colour g_colours_standard[MAX_WORMS] = {
	colour(s2, s0, s1, 1),
	colour(s1, s2, s0, 1),
	colour(s0, s1, s2, 1),
	colour(s2, s1, s0, 1),
	colour(s0, s2, s1, 1),
	colour(s1, s0, s2, 1),
};

colour g_colours_cb0[MAX_WORMS] = {
	colour(c1, c0, 0, 1),
	colour(c0, c1, 0, 1),
	colour(c1, c1, 0, 1),
	colour(c3, c2, 0, 1),
	colour(c2, c3, 0, 1),
	colour(c2, c2, 0, 1),
};

colour g_colours_cb1[MAX_WORMS] = {
	colour(0, c1, c0, 1),
	colour(0, c0, c1, 1),
	colour(0, c1, c1, 1),
	colour(0, c2, c3, 1),
	colour(0, c2, c2, 1),
	colour(0, c3, c2, 1),
};

#undef s0
#undef s1
#undef c0
#undef c1
#undef c2
#undef c3

colour g_colours[MAX_WORMS];

void init_colours() {
	memcpy(g_colours, g_colours_standard, sizeof(g_colours));
}

void toggle_colour_bind(map* m, player_state* ps) {
	switch(g_colour_mode = (g_colour_mode + 1) % 3) {
		case 0:	memcpy(g_colours, g_colours_standard, sizeof(g_colours)); break;
		case 1:	memcpy(g_colours, g_colours_cb0, sizeof(g_colours)); break;
		case 2:	memcpy(g_colours, g_colours_cb1, sizeof(g_colours)); break;
	}

	for(int i = 0; i < ps->num_worms; i++) {
		m->colours[i] = g_colours[i];
	}
}

void map::destroy() {
	delete [] data;
	size = ivec2();
	data = 0;
}

ivec2 measure_map(const uint8_t* p, const uint8_t* e) {
	ivec2 size(0, 0);

	for(; p < e; p++) {
		if ((*p != '#') && !(*p >= 'A' && *p <= 'Z'))
			break;

		int x = 0;

		while(p < e && *p != '\r' && *p != '\n') {
			x++;
			p++;
		}

		if ((p + 1) < e && ((*p == '\r' && p[1] == '\n') || (*p == '\n' && p[1] == '\r')))
			p++;

		size.x = Max(size.x, x);
		size.y++;
	}

	return size;
}

void set_map(map* m, player_state* ps, int x, int y, uint8_t c) {
	bool clear_tile = c == ' ';

	if (c >= '1' && c <= '9') {
		int num = c - '1';

		if (num < MAX_WORMS) {
			worm* w = &ps->worms[num];

			ps->num_worms = Max(ps->num_worms, num + 1);

			if (w->num_blocks < MAX_WORM_BLOCKS) {
				worm_block* b = &w->blocks[w->num_blocks++];

				b->pos = ivec2(x, y);
				b->age = ++w->age;
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

	if (clear_tile) {
		m->data[y * m->size.x + x] = TILE_EMPTY;
		m->tl.x = Min(m->tl.x, x);
		m->tl.y = Min(m->tl.y, y);
		m->br.x = Max(m->br.x, x + 1);
		m->br.y = Max(m->br.y, y + 1);
	}
}

bool load_map(map* m, player_state* ps, const char* path) {
	m->destroy();
	ps->reset();

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

	m->tl = m->size;
	m->br = ivec2();

	for(int y = 0; p < e; p++, y++) {
		if ((*p != '#') && !(*p >= 'A' && *p <= 'Z'))
			break;

		int x = 0;

		while(p < e && *p != '\r' && *p != '\n') {
			set_map(m, ps, x, y, *p);
			x++;
			p++;
		}

		if ((p + 1) < e && ((*p == '\r' && p[1] == '\n') || (*p == '\n' && p[1] == '\r')))
			p++;
	}

	for(int i = 0; i < ps->num_worms; i++) {
		ivec2 tp = m->targets[i];
		ivec2 dir;
		
		if (m->at(tp.x - 1, tp.y) == TILE_EMPTY) dir.x--;
		if (m->at(tp.x + 1, tp.y) == TILE_EMPTY) dir.x++;
		if (m->at(tp.x, tp.y - 1) == TILE_EMPTY) dir.y--;
		if (m->at(tp.x, tp.y + 1) == TILE_EMPTY) dir.y++;

		m->colours[i] = g_colours[i];
		m->target_dir[i] = dir;
	}

	return true;
}

void render_map(map* m, player_state* ps, map_effects* fx) {
	ivec2 size = m->size;
	colour c(Lerp(0.15f, 0.0f, Min(fx->win, 1.0f)), 1.0f - Min(fx->win, 1.0f));

	for(int y = 0; y < size.y; y++) {
		for(int x = 0; x < size.x; x++) {
			if (m->at(x, y) != TILE_EMPTY)
				continue;

			vec2 p0(to_vec2(ivec2(x, y)));
			vec2 p1(p0 + 1.0f);

			draw_rect(p0, p1, c);
		}
	}

	for(int i = 0; i < ps->num_worms; i++) {
		worm* w = ps->worms + i;

		// blocks

		colour fade;

		if (fx->win > 0.0f) {
			fade = colour(1.0f + Max(fx->win, Square(fx->win * 1.1f)), 1.0f);
		} else {
			float flash = fabsf(sinf((fx->pulse[i] / 0.2f) * PI * 2.0f)) * 0.25f;
			flash -= fx->selected[i] * 4.0f;
			fade = colour(1.0f - flash, 1.0f);
		}

		for(int j = 0; j < w->num_blocks; j++) {
			worm_block* b = w->blocks + j;

			vec2 p0(to_vec2(ivec2(b->pos)));
			vec2 p1(p0 + vec2(1.0f));

			draw_rect(p0, p1, m->colours[i] * fade);
		}

		// active block

		{
			worm_block* b = w->blocks + w->active_block;
			colour fade_off(Max(1.0f - fx->win, 0.0f));

			{
				vec2 p0(to_vec2(ivec2(b->pos)) + vec2(0.25f) + fx->jink[i]);
				vec2 p1(p0 + vec2(0.5f));
				colour col = (ps->active_worm == i) ? colour(0.0f, 0.55f) : colour(0.0f, 0.1f);

				draw_rect(p0, p1, col * fade_off);
			}

			if (ps->active_worm == i) {
				vec2 p0(to_vec2(ivec2(b->pos)) + vec2(0.375f) + fx->jink[i]);
				vec2 p1(p0 + vec2(0.25f));
				colour col = (ps->active_worm == i) ? colour(0.0f, 0.275f) : colour(0.0f, 0.1f);

				draw_rect(p0, p1, col * fade_off);
			}
		}

		// target

		{
			ivec2 tp = m->targets[i];
			ivec2 td = m->target_dir[i];
			vec2 p0(to_vec2(tp));
			vec2 p1(p0);
			colour c(m->colours[i]);
			float t = 0.5f;

			if (fx->win > 0.0f)
				c = c * (1.0f + Max(fx->win, Square(fx->win * 1.1f)));
			else
				c = c * colour(Lerp(0.35f, 1.0f, fx->target_active[i]), 0.65f);

			if (td.x < 0) { p0 += vec2(0.1f, 0.1f);		p1 += vec2(t, 0.9f); }
			if (td.x > 0) { p0 += vec2(1.0f - t, 0.1f);	p1 += vec2(0.9f, 0.9f); }
			if (td.y < 0) { p0 += vec2(0.1f, 0.1f);		p1 += vec2(0.9f, t); }
			if (td.y > 0) { p0 += vec2(0.1f, 1.0f - t);	p1 += vec2(0.9f, 0.9f); }

			draw_rect(p0, p1 , c);
		}
	}
}