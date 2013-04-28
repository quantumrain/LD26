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

int worm_at(player_state* ps, ivec2 pos) {
	for(int i = 0; i < ps->num_worms; i++) {
		if (block_at(ps->worms + i, pos) >= 0)
			return i;
	}

	return -1;
}

bool is_open_tile(map* m, player_state* ps, ivec2 pos) {
	if (m->at(pos.x, pos.y) != TILE_EMPTY)
		return false;

	return worm_at(ps, pos) < 0;
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

bool is_worm_split(map* m, player_state* ps, worm* w) {
	if (w->num_blocks <= 1)
		return false;

	int touched[MAX_WORM_BLOCKS] = { };

	int num_touched = 1;
	touched[w->active_block] = 1;

	for(;;) {
		int touched_this_pass = 0;

		for(int j = 0; j < w->num_blocks; j++) {
			if (touched[j] == 1) {
				worm_block* b = w->blocks + j;

				int u = block_at(w, b->pos + ivec2(0, -1));
				int d = block_at(w, b->pos + ivec2(0, 1));
				int l = block_at(w, b->pos + ivec2(-1, 0));
				int r = block_at(w, b->pos + ivec2(1, 0));

				if (u >= 0 && !touched[u]) { touched[u] = 1; num_touched++; touched_this_pass++; }
				if (d >= 0 && !touched[d]) { touched[d] = 1; num_touched++; touched_this_pass++; }
				if (l >= 0 && !touched[l]) { touched[l] = 1; num_touched++; touched_this_pass++; }
				if (r >= 0 && !touched[r]) { touched[r] = 1; num_touched++; touched_this_pass++; }

				touched[j] = 2;
			}
		}

		if (num_touched == w->num_blocks)
			break;

		if (touched_this_pass == 0)
			return true;
	}

	return false;
}

int anchor_block(map* m, player_state* ps, worm* w) {
	for(int j = 0; j < w->num_blocks; j++) {
		worm_block* b = w->blocks + j;

		ivec2 anchor_pos = b->pos + ivec2(0, 1);

		if (block_at(w, anchor_pos) >= 0)
			continue;

		if (!is_open_tile(m, ps, anchor_pos))
			return j;
	}

	return -1;
}

bool all_worms_anchored(map* m, player_state* ps) {
	for(int i = 0; i < ps->num_worms; i++) {
		worm* w = ps->worms + i;

		if (anchor_block(m, ps, w) < 0)
			return false;
	}

	return true;
}