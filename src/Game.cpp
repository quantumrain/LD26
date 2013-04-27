#include "Pch.h"
#include "Common.h"
#include "Game.h"

game_state g_gs;
map g_map;

void move_worm(map* m, game_state* gs, int mx, int my) {
	worm* w = gs->worms + gs->active_worm;

}

void update_game(map* m, game_state* gs) {
	if (gKey >= KEY_1 && gKey <= KEY_0) {
		int num = gKey - KEY_1;

		if (num < gs->num_worms) {
			gs->active_worm = num;
		}
	}

	if (gKey == KEY_FIRE) {
		gs->active_worm = (gs->active_worm + 1) % gs->num_worms;
	}

	if (gKey == KEY_LEFT) move_worm(m, gs, -1, 0);
	if (gKey == KEY_RIGHT) move_worm(m, gs, 1, 0);
	if (gKey == KEY_UP) move_worm(m, gs, 0, -1);
	if (gKey == KEY_DOWN) move_worm(m, gs, 0, 1);
}

void GameInit() {
	load_map(&g_map, &g_gs, "data\\map0.txt");
}

void GameUpdate() {
	update_game(&g_map, &g_gs);

	set_camera(vec2(g_map.size.x * 0.5f, g_map.size.y * -0.5f), 10.0f);

	render_map(&g_map, &g_gs, vec2(1.0f));
}