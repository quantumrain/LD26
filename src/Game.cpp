#include "Pch.h"
#include "Common.h"
#include "Game.h"

enum game_state {
	GS_PLAYING,
	GS_LEVEL_INTRO,
	GS_LEVEL_OUTRO,
	GS_RESET_LEVEL,
	GS_WIN_LEVEL,
};

game_state g_gs;
player_state g_ps;
map g_map;
int g_level_num = 0;
float g_state_time;

bool load_level(int num) {
	char buf[MAX_PATH];
	_snprintf_s(buf, MAX_PATH, _TRUNCATE, "data\\map%i.txt", num);
	return load_map(&g_map, &g_ps, buf);
}

void change_game_state(game_state new_state) {
	g_state_time = 0.0f;

	switch(g_gs = new_state) {
		case GS_PLAYING:
		break;

		case GS_LEVEL_INTRO:
		case GS_RESET_LEVEL:
			if (!load_level(g_level_num)) {
				DebugLn("Level missing!");
			}
		break;

		case GS_LEVEL_OUTRO:
		break;

		case GS_WIN_LEVEL:
			SoundPlay(kSid_Win, 1.0f, 1.0f); // TODO: Win effect
		break;
	}
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
	switch(g_gs) {
		case GS_PLAYING:
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

			if (gKey == KEY_CHEAT) {
				g_level_num++;
				change_game_state(GS_RESET_LEVEL);
			}

			if (gKey == KEY_RESET) {
				SoundPlay(kSid_Buzz, 0.5f, 1.0f); // TODO: Reset effect
				change_game_state(GS_RESET_LEVEL);
			}

			if (has_won(m, ps)) {
				change_game_state(GS_WIN_LEVEL);
			}
		break;

		case GS_LEVEL_INTRO:
			if ((g_state_time += DT) >= 0.75f) {
				change_game_state(GS_PLAYING);
			}
		break;

		case GS_LEVEL_OUTRO:
			if ((g_state_time += DT) >= 0.75f) {
				change_game_state(GS_LEVEL_INTRO);
			}
		break;

		case GS_RESET_LEVEL:
			change_game_state(GS_PLAYING);
		break;

		case GS_WIN_LEVEL:
			if ((gKey == KEY_FIRE) || ((g_state_time += DT) >= 1.0f)) {
				g_level_num++;
				change_game_state(GS_LEVEL_OUTRO);
			}
		break;
	}
}

void GameInit() {
	change_game_state(GS_LEVEL_INTRO);
}

void GameUpdate() {
	update_game(&g_map, &g_ps);

	vec2 size(to_vec2(g_map.br - g_map.tl));
	vec2 centre(to_vec2(g_map.br + g_map.tl) * vec2(0.5f, -0.5f));

	vec2 cam_pos = centre;
	float width = 10.0f;

	switch(g_gs) {
		case GS_LEVEL_INTRO:
			cam_pos.x += Lerp(0.0f, size.x + width, Square(1.0f - (g_state_time / 0.75f)));
		break;

		case GS_LEVEL_OUTRO:
			cam_pos.x -= Lerp(0.0f, size.x + width, Square(g_state_time / 0.75f));
		break;
	}

	set_camera(cam_pos, width);

	render_map(&g_map, &g_ps, vec2(1.0f, -1.0f));
}