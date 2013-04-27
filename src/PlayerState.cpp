#include "Pch.h"
#include "Common.h"
#include "Game.h"

void player_state::reset() {
	*this = player_state();
}

int block_at(worm* w, ivec2 pos) {
	for(int i = 0; i < w->num_blocks; i++) {
		worm_block* b = w->blocks + i;

		if (b->pos == pos)
			return i;
	}

	return -1;
}

bool is_open_tile(map* m, player_state* ps, ivec2 pos) {
	if (m->at(pos.x, pos.y) != TILE_EMPTY)
		return false;

	for(int i = 0; i < ps->num_worms; i++) {
		if (block_at(ps->worms + i, pos) >= 0)
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

bool is_player_valid(map* m, player_state* ps) {
	for(int i = 0; i < ps->num_worms; i++) {
		worm* w = ps->worms + i;

		// worm has an anchor to the ground

		bool has_anchor = false;

		for(int j = 0; j < w->num_blocks; j++) {
			worm_block* b = w->blocks + j;

			ivec2 anchor_pos = b->pos + ivec2(0, 1);

			if (block_at(w, anchor_pos) >= 0)
				continue;

			if (!is_open_tile(m, ps, anchor_pos))
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