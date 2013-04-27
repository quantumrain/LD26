#include "Pch.h"
#include "Common.h"
#include "Game.h"

game_state g_gs;
map g_map;

void set_active_worm(game_state* gs, int num) {
	SoundPlay(kSid_Dead, 0.75f + num * 0.15f, 0.75f);
	gs->active_worm = num;
}

int block_at(worm* w, ivec2 pos) {
	for(int i = 0; i < w->num_blocks; i++) {
		worm_block* b = w->blocks + i;

		if (b->pos == pos)
			return i;
	}

	return -1;
}

bool is_open_tile(map* m, game_state* gs, ivec2 pos) {
	if (m->at(pos.x, pos.y) != TILE_EMPTY)
		return false;

	for(int i = 0; i < gs->num_worms; i++) {
		if (block_at(gs->worms + i, pos) >= 0)
			return false;
	}

	return true;
}

int next_oldest_block(worm* w, int min_age) {
	int best = -1;
	int best_age = INT_MAX;

	for(int i = 0; i < w->num_blocks; i++) {
		worm_block* b = w->blocks + i;

		if (min_age < b->age && b->age < best_age) {
			best = i;
			best_age = b->age;
		}
	}

	return best;
}

bool is_map_valid(map* m, game_state* gs) {
	for(int i = 0; i < gs->num_worms; i++) {
		worm* w = gs->worms + i;

		// worm has an anchor to the ground

		bool has_anchor = false;

		for(int j = 0; j < w->num_blocks; j++) {
			worm_block* b = w->blocks + j;

			ivec2 anchor_pos = b->pos + ivec2(0, 1);

			if (block_at(w, anchor_pos) >= 0)
				continue;

			if (!is_open_tile(m, gs, anchor_pos))
				has_anchor = true;
		}

		if (!has_anchor)
			return false;

		// worm hasn't been split

		if (w->num_blocks > 1) {
			int touched[MAX_WORM_BLOCKS] = { };

			bool has_updated = true;
			int num_touched = 1;
			touched[w->active_block] = 1;

			for(;; has_updated = false) {
				for(int j = 0; j < w->num_blocks; j++) {
					if (touched[j] == 1) {
						worm_block* b = w->blocks + j;

						int u = block_at(w, b->pos + ivec2(0, -1));
						int d = block_at(w, b->pos + ivec2(0, 1));
						int l = block_at(w, b->pos + ivec2(-1, 0));
						int r = block_at(w, b->pos + ivec2(1, 0));

						if (u >= 0 && !touched[u]) { has_updated |= true; touched[u] = 1; num_touched++; }
						if (d >= 0 && !touched[d]) { has_updated |= true; touched[d] = 1; num_touched++; }
						if (l >= 0 && !touched[l]) { has_updated |= true; touched[l] = 1; num_touched++; }
						if (r >= 0 && !touched[r]) { has_updated |= true; touched[r] = 1; num_touched++; }

						touched[j] = 2;
					}
				}

				if (num_touched == w->num_blocks)
					break;

				if (!has_updated)
					return false;
			}
		}
	}

	return true;
}

void move_worm(map* m, game_state* gs, ivec2 dir) {
	worm* w = gs->worms + gs->active_worm;
	worm_block* curr_block = w->blocks + w->active_block;
	ivec2 target_pos = curr_block->pos + dir;

	int existing = block_at(w, target_pos);

	if (existing >= 0) {
		SoundPlay(kSid_Jump, 2.0f, 0.25f);
		w->active_block = existing;
		return;
	}

	if (!is_open_tile(m, gs, target_pos)) {
		SoundPlay(kSid_Jump, 0.5f, 0.25f); // TODO: Blocked effect
		return;
	}

	int age = 0;

	for(;;) {
		int candidate = next_oldest_block(w, age);

		if (candidate < 0) {
			SoundPlay(kSid_Jump, 0.5f, 0.25f); // TODO: No candidates effect
			return;
		}

		worm_block* b = w->blocks + candidate;

		game_state old_gs = *gs;
		age = b->age;

		w->active_block = candidate;
		b->pos = target_pos;
		b->age = ++w->age;

		if (is_map_valid(m, gs)) {
			SoundPlay(kSid_Jump, 1.5f, 0.25f); // TODO: Success effect
			break;
		}

		*gs = old_gs;
	}
}

void update_game(map* m, game_state* gs) {
	if (gKey >= KEY_1 && gKey <= KEY_0) {
		int num = gKey - KEY_1;
		if (num < gs->num_worms) set_active_worm(gs, num);
	}
		
	if (gKey == KEY_FIRE) set_active_worm(gs, (gs->active_worm + 1) % gs->num_worms);
	if (gKey == KEY_ALT_FIRE) set_active_worm(gs, (gs->active_worm + gs->num_worms - 1) % gs->num_worms);

	if (gKey == KEY_LEFT) move_worm(m, gs, ivec2(-1, 0));
	if (gKey == KEY_RIGHT) move_worm(m, gs, ivec2(1, 0));
	if (gKey == KEY_UP) move_worm(m, gs, ivec2(0, -1));
	if (gKey == KEY_DOWN) move_worm(m, gs, ivec2(0, 1));
}

void GameInit() {
	load_map(&g_map, &g_gs, "data\\map0.txt");
}

void GameUpdate() {
	update_game(&g_map, &g_gs);

	set_camera(vec2(g_map.size.x * 0.5f, g_map.size.y * -0.5f), 10.0f);

	render_map(&g_map, &g_gs, vec2(1.0f));
}