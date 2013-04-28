#include "Pch.h"
#include "Common.h"
#include "Game.h"

enum game_state {
	GS_TITLE,
	GS_COMPLETE,
	GS_PLAYING,
	GS_LEVEL_INTRO,
	GS_LEVEL_OUTRO,
	GS_RESET_LEVEL,
	GS_WIN_LEVEL,
	GS_WIN_GAME
};

game_state g_gs;
player_state g_ps;
map_effects g_map_fx;
map g_map;
int g_level_num = 0;
float g_state_time;

bool load_level(int num) {
	char buf[MAX_PATH];
	_snprintf_s(buf, MAX_PATH, _TRUNCATE, "data\\map%i.txt", num);
	g_map_fx.reset();
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
				Panic("Missing map file!");
			}
		break;

		case GS_LEVEL_OUTRO:
		break;

		case GS_WIN_LEVEL:
			SoundPlay(kSid_Win, 1.0f, 1.0f);
		break;

		case GS_WIN_GAME:
			// TODO: Win game!
		break;
	}
}

void set_active_worm(player_state* ps, map_effects* fx, int num) {
	if (ps->active_worm != num) {
		SoundPlay(kSid_Switch, 0.75f + num * 0.15f, 0.75f);
		ps->active_worm = num;
		fx->selected[num] = 0.25f;
	}
}

void move_worm(map* m, player_state* ps, map_effects* fx, ivec2 dir) {
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
		SoundPlay(kSid_Dit, 0.5f, 0.25f);
		int cw = worm_at(ps, target_pos);
		spawn_effect(EFFECT_COLLIDE, curr_block->pos, dir, (cw >= 0) ? m->colours[cw] : colour(0.0f, 1.0f));
		fx->pulse[ps->active_worm] = 0.2f;
		fx->jink[ps->active_worm] = to_vec2(dir) * 0.25f;
		return;
	}

	int age = 0;
	
	const int MAX_BAD_ANCHORS = MAX_WORMS * 2;
	ivec2 bad_anchors[MAX_BAD_ANCHORS];
	int bad_anchor_worm[MAX_BAD_ANCHORS];
	int num_bad_anchors = 0;

	for(;;) {
		int candidate = next_oldest_block(w, age);

		if (candidate < 0) {
			SoundPlay(kSid_Buzz, 0.75f, 0.25f);

			for(int i = 0; i < num_bad_anchors; i++) {
				int wi = bad_anchor_worm[i];
				spawn_effect(EFFECT_ANCHOR_FLASH, bad_anchors[i], ivec2(1, 0), m->colours[wi]);
				fx->pulse[wi] = 0.2f;
			}

			fx->jink[ps->active_worm] = to_vec2(dir) * 0.25f;

			return;
		}

		worm_block* b = w->blocks + candidate;

		player_state old_ps = *ps;
		age = b->age;

		w->active_block = candidate;
		b->pos = target_pos;
		b->age = ++w->age;

		if (!is_worm_split(m, ps, w)) {
			if (all_worms_anchored(m, ps)) { // TODO: Needs to check that worms aren't supporting each other
				SoundPlay(kSid_Dit, 1.5f, 0.25f);
				break;
			}

			for(int i = 0; i < ps->num_worms; i++) {
				worm* w = ps->worms + i;

				if (anchor_block(m, ps, w) >= 0)
					continue;

				if (num_bad_anchors < MAX_BAD_ANCHORS) {
					worm* old_w = old_ps.worms + i;
					int anchor = anchor_block(m, &old_ps, old_w);

					if (anchor >= 0) {
						bad_anchors[num_bad_anchors] = old_w->blocks[anchor].pos;
						bad_anchor_worm[num_bad_anchors] = i;
						num_bad_anchors++;
					}
				}
			}

		}

		*ps = old_ps;
	}
}

int find_next_best_worm(map* m, player_state* ps, ivec2 p, bool forward) {
	int best = ps->active_worm;
	int best_score = forward ? INT_MAX : 0;

	int pivot_score = (m->size.y - p.y) + p.x * m->size.y;
	int max_score = m->size.x * m->size.y;

	for(int i = 0; i < ps->num_worms; i++) {
		if (i == ps->active_worm)
			continue;

		worm* w = ps->worms + i;
		ivec2 ap = w->blocks[w->active_block].pos;

		int score = ((m->size.y - ap.y) + ap.x * m->size.y - pivot_score + max_score) % max_score;

		if ((forward && score < best_score) || (!forward && score > best_score)) {
			best = i;
			best_score = score;
		}
	}

	return best;
}

void update_game_play(map* m, player_state* ps, map_effects* fx) {
	if (gKey >= KEY_1 && gKey <= KEY_0) {
		int num = gKey - KEY_1;
		if (num < ps->num_worms) set_active_worm(ps, fx, num);
	}

	if (gKey == KEY_FIRE) {
		set_active_worm(ps, fx, (ps->active_worm + 1) % ps->num_worms);
		//worm* w = ps->worms + ps->active_worm;
		//set_active_worm(ps, find_next_best_worm(m, ps, w->blocks[w->active_block].pos, true));
	}

	if (gKey == KEY_ALT_FIRE) {
		set_active_worm(ps, fx, (ps->active_worm + ps->num_worms - 1) % ps->num_worms);
		//worm* w = ps->worms + ps->active_worm;
		//set_active_worm(ps, find_next_best_worm(m, ps, w->blocks[w->active_block].pos, false));
	}

	if (gKey == KEY_LEFT) move_worm(m, ps, fx, ivec2(-1, 0));
	if (gKey == KEY_RIGHT) move_worm(m, ps, fx, ivec2(1, 0));
	if (gKey == KEY_UP) move_worm(m, ps, fx, ivec2(0, -1));
	if (gKey == KEY_DOWN) move_worm(m, ps, fx, ivec2(0, 1));

	if (gKey == KEY_CHEAT) {
		g_level_num++;
		change_game_state(GS_RESET_LEVEL);
	}

	if (gKey == KEY_RESET) {
		SoundPlay(kSid_Buzz, 0.5f, 1.0f); // TODO: Reset effect
		change_game_state(GS_RESET_LEVEL);
	}

	if (gKey == KEY_MODE) toggle_colour_bind(&g_map, &g_ps);

	// Mouse control, easier to use, but flaky right now
	/*static bool was_down;

	if (gMouseButtons & 1) {
		vec2 fp = to_game((to_vec2(gMousePos) / to_vec2(g_WinSize)) * 2.0f - 1.0f);
		ivec2 p((int)fp.x, (int)fp.y);

		static ivec2 first_pos;
		static int first_worm;

		if (!was_down) {
			first_pos = p;
			was_down = true;

			first_worm = worm_at(&g_ps, first_pos);

			if (first_worm >= 0) {
				set_active_worm(&g_ps, &g_map_fx, first_worm);
			}
		}

		if (first_worm >= 0) {
			worm* w = g_ps.worms + first_worm;
			int bi = block_at(w, p);

			if (bi >= 0) {
				if (w->active_block != bi) {
					SoundPlay(kSid_Dit, 2.0f, 0.25f);
					w->active_block = bi;
					w->blocks[bi].age = ++w->age;
				}

				first_pos = p;
			} else {
				if (first_pos != p) {
					ivec2 ap = w->blocks[w->active_block].pos;

					if (p != ap) {
						if (p.x < ap.x) move_worm(m, ps, fx, ivec2(-1, 0));
						if (p.x > ap.x) move_worm(m, ps, fx, ivec2(1, 0));
						if (p.y < ap.y) move_worm(m, ps, fx, ivec2(0, -1));
						if (p.y > ap.y) move_worm(m, ps, fx, ivec2(0, 1));
					}
				}

				first_pos = p;
			}
		}
	} else {
		was_down = false;
	}*/

	if (has_won(m, ps)) {
		change_game_state(GS_WIN_LEVEL);
	}
}

void update_game(map* m, player_state* ps, map_effects* fx) {
	switch(g_gs) {
		case GS_TITLE:
			if (gKey == KEY_FIRE)
				g_state_time = Max(g_state_time, 3.0f);

			if (gKey == KEY_CHEAT)
				change_game_state(GS_LEVEL_INTRO);

			if ((g_state_time += DT) > 4.0f)
				change_game_state(GS_LEVEL_INTRO);
		break;

		case GS_COMPLETE:
			if ((g_state_time += DT) > 4.0f)
				PostQuitMessage(0);
		break;

		case GS_PLAYING:
			update_game_play(m, ps, fx);
		break;

		case GS_LEVEL_INTRO:
			if (g_state_time > 0.75f)
				update_game_play(m, ps, fx);

			if ((g_state_time += DT) >= 2.0f) {
				change_game_state(GS_PLAYING);
			}
		break;

		case GS_LEVEL_OUTRO:
			g_map_fx.win += DT;
			if ((g_state_time += DT) >= 1.5f) {
				if (g_level_num >= 10)
					change_game_state(GS_COMPLETE);
				else
					change_game_state(GS_LEVEL_INTRO);
			}
		break;

		case GS_RESET_LEVEL:
			change_game_state(GS_PLAYING);
		break;

		case GS_WIN_LEVEL:
			g_map_fx.win += DT;
			if ((gKey == KEY_FIRE) || ((g_state_time += DT) >= 1.0f)) {
				g_level_num++;
				change_game_state(GS_LEVEL_OUTRO);
			}
		break;
	}
}

void GameInit() {
	void init_colours();
	init_colours();
	change_game_state(GS_TITLE);
}

void GameUpdate() {
	update_game(&g_map, &g_ps, &g_map_fx);
	update_map_effects(&g_map, &g_ps, &g_map_fx);
	update_effects();

	if (g_gs == GS_TITLE) {
		set_camera(vec2(), 10.0f);
		draw_string(vec2(0.0f, -1.0f), vec2(0.1f), TEXT_CENTRE, colour(), "Gravity Worm");
		draw_string(vec2(0.0f, 3.0f), vec2(0.04f), TEXT_CENTRE, colour(0.5f), "Created for Ludum Dare 26: Minimalism");
		draw_string(vec2(0.0f, 3.5f), vec2(0.04f), TEXT_CENTRE, colour(0.5f), "by Stephen Cakebread");
		draw_string(vec2(0.0f, 4.0f), vec2(0.04f), TEXT_CENTRE, colour(0.5f), "t: @quantumrain");
	} else if (g_gs == GS_COMPLETE) {
		set_camera(vec2(), 10.0f);
		draw_string(vec2(0.0f, -1.0f), vec2(0.1f), TEXT_CENTRE, colour(), "Success");
	} else {
		vec2 size(to_vec2(g_map.br - g_map.tl));
		vec2 centre(to_vec2(g_map.br + g_map.tl) * vec2(0.5f, 0.5f));

		vec2 cam_pos = centre;
		float width = 10.0f;

		set_camera(cam_pos, width);
		render_map(&g_map, &g_ps, &g_map_fx);
		render_effects();
	}

	switch(g_gs) {
		case GS_TITLE:
			set_tint(colour(1.0f - Max(g_state_time - 3.0f, 0.0f)));
		break;

		case GS_COMPLETE:
			set_tint(colour(Max(g_state_time - 0.5f, 0.0f) / 1.5f));
		break;

		case GS_LEVEL_INTRO:
			set_tint(colour(Max(g_state_time - 0.5f, 0.0f) / 1.5f));
		break;

		case GS_LEVEL_OUTRO:
			set_tint(colour(1.0f - (g_state_time / 1.5f)));
		break;

		default:
			set_tint(colour());
		break;
	}
}