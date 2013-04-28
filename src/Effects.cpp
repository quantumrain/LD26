#include "Pch.h"
#include "Common.h"
#include "Game.h"

// map effects

map_effects::map_effects() {
	reset();
}

void map_effects::reset() {
	for(int i = 0; i < MAX_WORMS; i++) {
		pulse[i] = 0.0f;
		selected[i] = 0.0f;
		jink[i] = vec2();
		target_active[i] = 0.0f;
	}

	win = 0.0f;
}

void update_map_effects(map* m, player_state* ps, map_effects* fx) {
	for(int i = 0; i < ps->num_worms; i++) {
		float& f = fx->pulse[i];
		float& s = fx->selected[i];
		vec2& j = fx->jink[i];
		float& t = fx->target_active[i];

		f = Max(f - DT, 0.0f);
		s = Max(s - DT, 0.0f);
		j *= 0.8f;

		t = Lerp(t, (block_at(ps->worms + i, m->targets[i] + m->target_dir[i]) >= 0) ? 1.0f : 0.0f, 0.2f);
	}
}

// general effects

struct effect {
	effect_type type;
	float time;
	float lifetime;
	ivec2 pos;
	ivec2 dir;
	colour col;

	effect() : type(EFFECT_NONE), time(), lifetime() { }
};

const int MAX_EFFECTS = 32;
effect g_effects[MAX_EFFECTS];
int g_num_effects;

void spawn_effect(effect_type type, ivec2 pos, ivec2 dir, colour col) {
	if (g_num_effects >= MAX_EFFECTS)
		return;

	effect* e = g_effects + g_num_effects++;

	e->type = type;
	e->time = 0.0f;
	e->lifetime = 0.25f;
	e->pos = pos;
	e->dir = dir;
	e->col = col;
}

void update_effects() {
	for(int i = 0; i < g_num_effects; i++) {
		effect* e = g_effects + i;

		if ((e->time += DT) >= e->lifetime) {
			*e = g_effects[--g_num_effects];
			i--;
			continue;
		}
	}
}

void render_effects() {
	for(int i = 0; i < g_num_effects; i++) {
		effect* e = g_effects + i;
		float f = e->time / e->lifetime;

		switch(e->type) {
			case EFFECT_ANCHOR_FLASH: {
				vec2 pos = to_vec2(e->pos);

				float inv_f = 1.0f - Square(f);

				colour c0(0.0f);
				colour c1(e->col.r * inv_f, e->col.g * inv_f, e->col.b * inv_f, 0.0f);
				colour c2(e->col * inv_f);

				draw_rect(pos + vec2(0.0f, 0.85f), pos + vec2(1.0f, 1.0f), c0, c0, c1, c1);
				draw_rect(pos + vec2(0.0f, 1.0f), pos + vec2(1.0f, 1.1f), c2, c2, c0, c0);
			}
			break;

			case EFFECT_COLLIDE: {
				vec2 pos = to_vec2(e->pos);

				float inv_f = 1.0f - Square(f);

				colour c0(0.0f);
				colour c1(e->col.r * inv_f, e->col.g * inv_f, e->col.b * inv_f, inv_f * 0.75f);

				if (e->dir.x < 0) draw_rect(pos + vec2(0.0f, 0.0f), pos + vec2(0.1f, 1.0f), c1, c0, c1, c0);
				if (e->dir.x > 0) draw_rect(pos + vec2(0.9f, 0.0f), pos + vec2(1.0f, 1.0f), c0, c1, c0, c1);
				if (e->dir.y < 0) draw_rect(pos + vec2(0.0f, 0.0f), pos + vec2(1.0f, 0.1f), c1, c1, c0, c0);
				if (e->dir.y > 0) draw_rect(pos + vec2(0.0f, 0.9f), pos + vec2(1.0f, 1.0f), c0, c0, c1, c1);
			}
			break;
		}
	}
}
