#include "Pch.h"
#include "Common.h"
#include "Game.h"

player_state g_ps;
map g_map;
int g_level_num = 1;

bool load_level(int num) {
	char buf[MAX_PATH];
	_snprintf_s(buf, MAX_PATH, _TRUNCATE, "data\\map%i.txt", num);
	return load_map(&g_map, &g_ps, buf);
}

void win_game() {
	DebugLn("You Win!");
	g_level_num = 0;
	load_level(g_level_num);
}

void set_active_worm(player_state* ps, int num) {
	SoundPlay(kSid_Switch, 0.75f + num * 0.15f, 0.75f);
	ps->active_worm = num;
}

void move_worm(map* m, player_state* ps, ivec2 dir) {
	worm* w = ps->worms + ps->active_worm;
	worm_block* curr_block = w->blocks + w->active_block;
	ivec2 target_pos = curr_block->pos + dir;

	int existing = block_at(w, target_pos);

	if (existing >= 0) {
		SoundPlay(kSid_Dit, 2.0f, 0.25f); // TODO: Cursor effect
		w->active_block = existing;
		w->blocks[existing].age = ++w->age;
		return;
	}

	if (!is_open_tile(m, ps, target_pos)) {
		SoundPlay(kSid_Dit, 0.5f, 0.25f); // TODO: Blocked effect
		return;
	}

	int age = 0;

	for(;;) {
		int candidate = next_oldest_block(w, age);

		if (candidate < 0) {
			SoundPlay(kSid_Buzz, 0.75f, 0.25f); // TODO: No candidates effect
			return;
		}

		worm_block* b = w->blocks + candidate;

		player_state old_ps = *ps;
		age = b->age;

		w->active_block = candidate;
		b->pos = target_pos;
		b->age = ++w->age;

		if (is_player_valid(m, ps)) {
			SoundPlay(kSid_Dit, 1.5f, 0.25f); // TODO: Success effect
			break;
		}

		*ps = old_ps;
	}
}

bool has_won(map* m, player_state* ps) {
	for(int i = 0; i < ps->num_worms; i++) {
		worm* w = ps->worms + i;

		if (
			(block_at(w, m->targets[i] + ivec2(-1, 0)) < 0) &&
			(block_at(w, m->targets[i] + ivec2(1, 0)) < 0) &&
			(block_at(w, m->targets[i] + ivec2(0, -1)) < 0) &&
			(block_at(w, m->targets[i] + ivec2(0, 1)) < 0)
		) {
			return false;
		}
	}

	return true;
}

void update_game(map* m, player_state* ps) {
	if (gKey >= KEY_1 && gKey <= KEY_0) {
		int num = gKey - KEY_1;
		if (num < ps->num_worms) set_active_worm(ps, num);
	}
		
	if (gKey == KEY_FIRE) set_active_worm(ps, (ps->active_worm + 1) % ps->num_worms);
	if (gKey == KEY_ALT_FIRE) set_active_worm(ps, (ps->active_worm + ps->num_worms - 1) % ps->num_worms);

	if (gKey == KEY_LEFT) move_worm(m, ps, ivec2(-1, 0));
	if (gKey == KEY_RIGHT) move_worm(m, ps, ivec2(1, 0));
	if (gKey == KEY_UP) move_worm(m, ps, ivec2(0, -1));
	if (gKey == KEY_DOWN) move_worm(m, ps, ivec2(0, 1));

	if (gKey == KEY_RESET) {
		SoundPlay(kSid_Buzz, 0.5f, 1.0f); // TODO: Reset effect
		load_level(g_level_num);
	}

	if (has_won(m, ps)) {
		SoundPlay(kSid_Win, 1.0f, 1.0f); // TODO: Win effect
		g_level_num++;

		if (!load_level(g_level_num)) {
			win_game();
		}
	}
}

void GameInit() {
	if (!load_level(g_level_num)) {
		DebugLn("Working directory problem!");
	}
}

void GameUpdate() {
	update_game(&g_map, &g_ps);

	set_camera(vec2(g_map.size.x * 0.5f, g_map.size.y * -0.5f), 10.0f);

	render_map(&g_map, &g_ps, vec2(1.0f));
}