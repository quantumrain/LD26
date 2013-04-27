#include "Pch.h"
#include "Common.h"

void GameInit()
{
}

void GameUpdate()
{
	static vec2 p(2, 2);
	static float w = 4.0f;

	p.x += (gKeyRight - gKeyLeft) * 0.1f;
	p.y += (gKeyUp - gKeyDown) * 0.1f;
	w += gKeyFire * 0.1f;

	SetCamera(p, w);

	DrawRect(vec2(1, 1), vec2(3, 3), colour(1, 1, 1, 1));
}