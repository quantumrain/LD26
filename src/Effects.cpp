#include "Pch.h"
#include "Common.h"
#include "Game.h"

enum effect_type {
	EFFECT_NONE,
	EFFECT_ANCHOR_FLASH
};

struct effect {
	effect_type type;
	float time;
	float lifetime;
	ivec2 pos;
	colour col;

	effect() : type(EFFECT_NONE), time(), lifetime() { }
};

const int MAX_EFFECTS = 32;
effect g_effects[MAX_EFFECTS];
int g_num_effects;

void effect_anchor_flash(ivec2 pos, colour col) {
	if (g_num_effects >= MAX_EFFECTS)
		return;

	effect* e = g_effects + g_num_effects++;

	e->type = EFFECT_ANCHOR_FLASH;
	e->time = 0.0f;
	e->lifetime = 0.25f;
	e->pos = pos + ivec2(0, 1);
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

				draw_rect(pos - vec2(0.0f, 0.25f), pos + vec2(1.0f, 0.0f), c0, c0, c1, c1);
				draw_rect(pos + vec2(0.0f, 0.0f), pos + vec2(1.0f, 0.1f), c2, c2, c0, c0);
			}
			break;
		}
	}
}
