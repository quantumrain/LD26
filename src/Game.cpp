#include "Pch.h"
#include "Common.h"
#include "Game.h"

game_state g_gs;
map g_map;

void GameInit() {
	load_map(&g_map, "data\\map0.txt");
}

void GameUpdate() {
	set_camera(vec2(g_map.size.x * 0.5f, g_map.size.y * -0.5f), 10.0f);

	render_map(&g_map, vec2(1.0f));
}