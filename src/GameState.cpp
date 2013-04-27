#include "Pch.h"
#include "Common.h"
#include "Game.h"

void game_state::reset() {
	*this = game_state();
}