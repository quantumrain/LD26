/*
###########
.#........# nice left hand wall with full spikes on other?
.#K.......#
.#........#
K#>.......#
.#>.......#
K#>......K#
.#........#
K#.......<#
.#.......<#
K#K......<#
.#........#
K#>.......#
.#>.......#
K#>.......#
.P........#
###########
*/

// Copyright 2012 Stephen Cakebread

#include "Pch.h"
#include "Common.h"

extern int kWinWidth;
extern int kWinHeight;

float FRand(float mag)
{
	return ((float)rand() / (float)RAND_MAX) * mag;
}

float SignedFRand(float mag)
{
	return FRand(2.0f * mag) - mag;
}

Vector2 RandBox(float magX, float magY)
{
	return Vector2(SignedFRand(magX), SignedFRand(magY));
}

bool OverlapsRect(Vector2 c0, Vector2 s0, Vector2 c1, Vector2 s1)
{
	Vector2 delta = c1 - c0;
	Vector2 size = (s0 + s1) * 0.5f;
	
	return (fabsf(delta.x) < size.x) && (fabsf(delta.y) < size.y);
}

struct Particle
{
	Vector2 position;
	Vector2 velocity;

	float time;
	float maxTime;

	Colour colour0;
	Colour colour1;

	float scale0;
	float scale1;
};

struct ParticleSystem
{
	int count;
	int max;
	Particle* particles;

	ParticleSystem() : count(0), max(0), particles(0) { }
	~ParticleSystem() { delete [] particles; }
};

void InitParticleSystem(ParticleSystem* sys, int max)
{
	if ((sys->particles = new Particle[max]) != 0)
	{
		sys->count = 0;
		sys->max = max;
	}
}

void UpdateParticleSystem(ParticleSystem* sys, Vector2 pan)
{
	float dt = 1.0f / 60.0f;

	int count = sys->count;

	for(int i = 0; i < count; )
	{
		Particle* p = &sys->particles[i];

		if ((p->time += dt) >= p->maxTime)
		{
			if (i < --count)
				*p = sys->particles[count];

			continue;
		}

		float t = p->time / p->maxTime;

		p->position += p->velocity * dt;

		DrawSprite(p->position + pan, Lerp(p->scale0, p->scale1, t), 255, 0, Lerp(p->colour0, p->colour1, t));

		i++;
	}

	sys->count = count;
}

void SpawnParticle(ParticleSystem* sys, Vector2 position, Vector2 velocity, float maxTime, Colour colour0, Colour colour1, float scale0, float scale1)
{
	if (sys->count >= sys->max)
		return;

	Particle* p = &sys->particles[sys->count++];

	p->position = position;
	p->velocity = velocity;

	p->time = 0.0f;
	p->maxTime = maxTime;

	p->colour0 = colour0;
	p->colour1 = colour1;

	p->scale0 = scale0 / 16.0f;
	p->scale1 = scale1 / 16.0f;
}

ParticleSystem gParticles;

void SpawnDiveCloud(Vector2 position)
{
	for(int i = 0; i < 20; i++)
	{
		SpawnParticle(&gParticles, position + Vector2(SignedFRand(7.0f), 1.0f), RandBox(35.0f, 7.5f) + Vector2(0.0f, -7.5f), 0.2f, Colour(0.75f, 1.0f), Colour(0.75f, 1.0f), 0.5f, 1.5f);
	}
}

void SpawnLandCloud(Vector2 position)
{
	for(int i = 0; i < 10; i++)
	{
		SpawnParticle(&gParticles, position + Vector2(SignedFRand(5.0f), 1.0f), RandBox(10.0f, 5.0f) + Vector2(0.0f, -5.0f), 0.2f, Colour(0.5f, 1.0f), Colour(0.5f, 1.0f), 0.5f, 1.0f);
	}
}

void SpawnJumpPuff(Vector2 position, int wallJump)
{
	if (wallJump < 0)
	{
		for(int i = 0; i < 5; i++)
			SpawnParticle(&gParticles, position + Vector2(-6.0f, SignedFRand(3.0f)), RandBox(5.0f, 10.0f) + Vector2(5.0f, 0.0f), 0.2f, Colour(0.5f, 1.0f), Colour(0.5f, 1.0f), 0.5f, 1.0f);
	}
	else if (wallJump > 0)
	{
		for(int i = 0; i < 5; i++)
			SpawnParticle(&gParticles, position + Vector2(6.0f, SignedFRand(3.0f)), RandBox(5.0f, 10.0f) + Vector2(-5.0f, 0.0f), 0.2f, Colour(0.5f, 1.0f), Colour(0.5f, 1.0f), 0.5f, 1.0f);
	}
	else
	{
		for(int i = 0; i < 5; i++)
			SpawnParticle(&gParticles, position + Vector2(SignedFRand(5.0f), 1.0f), RandBox(10.0f, 10.0f), 0.2f, Colour(0.5f, 1.0f), Colour(0.5f, 1.0f), 0.5f, 1.0f);
	}
}

struct Tile
{
	enum Kind
	{
		kEmpty,
		kWall,
		kCheckPoint,
		kKitten,
		kSpikesUp,
		kSpikesDown,
		kSpikesLeft,
		kSpikesRight,
		kDownOnly
	};

	Kind kind;

	Tile() : kind(kWall) { }
};

struct CollisionResult
{
	Tile* tile;
	int px, py;

	CollisionResult() : tile(0), px(-1), py(-1) { }
	CollisionResult(Tile* tile_, int px_, int py_) : tile(tile_), px(px_), py(py_) { }

	operator bool() const { return tile != 0; }
};

struct Map
{
	int _width;
	int _height;
	Tile* _tiles;
	int _playerSpawnX, _playerSpawnY;
	int _totalKittens;

	Map() : _width(0), _height(0), _tiles(0), _playerSpawnX(0), _playerSpawnY(0), _totalKittens(0) { }
	~Map() { Clear(); }

	bool Init(int width, int height)
	{
		Clear();

		if ((width < 1) || (height < 1))
			return false;

		if ((_tiles = new Tile[width * height]) == 0)
			return false;

		_width = width;
		_height = height;

		return true;
	}

	void Clear()
	{
		delete [] _tiles;

		_width = 0;
		_height = 0;
		_tiles = 0;
		_playerSpawnX = 0;
		_playerSpawnY = 0;
		_totalKittens = 0;
	}

	Tile* At(int x, int y)
	{
		if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
			return 0;

		return &_tiles[x + (y * _width)];
	}

	Tile* AtPx(int x, int y)
	{
		return At(x >> 4, y >> 4);
	}

	Tile::Kind AtKind(int x, int y)
	{
		if (Tile* tile = At(x, y))
			return tile->kind;

		return Tile::kWall;
	}

	bool CollidePt(int px, int py, bool movingUp)
	{
		if (Tile* t = AtPx(px, py))
		{
			if (movingUp && (t->kind == Tile::kDownOnly))
				return true;

			return t->kind == Tile::kWall;
		}

		return false;
	}

	CollisionResult HCollide(int px0, int px1, int py)
	{
		int dist = px1 - px0;
		int dlt = 1;

		if (dist < 0)
		{
			dist = -dist;
			dlt = -1;
		}

		for(int x = px0; dist >= 0; dist--, x += dlt) // TODO: Test only tile edges
		{
			if (CollidePt(x, py, false))
				return CollisionResult(AtPx(x, py), x, py);
		}

		return CollisionResult();
	}

	CollisionResult VCollide(int px, int py0, int py1)
	{
		int dist = py1 - py0;
		int dlt = 1;

		if (dist < 0)
		{
			dist = -dist;
			dlt = -1;
		}

		bool movingUp = (py1 < py0);

		for(int y = py0; dist >= 0; dist--, y += dlt) // TODO: Test only tile edges
		{
			if (CollidePt(px, y, movingUp))
				return CollisionResult(AtPx(px, y), px, y);
		}

		return CollisionResult();
	}
};

bool LoadMap(Map* map, const char* path)
{
	map->Clear();

	HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (h == INVALID_HANDLE_VALUE)
		return false;

	int length = GetFileSize(h, 0);

	const char* data = new char[length];

	if (!data)
	{
		CloseHandle(h);
		return false;
	}

	DWORD bytes;
	ReadFile(h, (void*)data, length, &bytes, 0);
	CloseHandle(h);

	int width = 0;
	int height = 1;

	for(int i = 0, x = 0; i < length; i++)
	{
		if ((data[i] == '\r') || (data[i] == '\n'))
		{
			width = Max(width, x);
			height++;
			x = 0;

			if (((i + 1) < length) && ((data[i + 1] == '\r') || (data[i + 1] == '\n')))
				i++;
		}
		else
			x++;
	}

	if (!map->Init(width, height))
	{
		delete [] data;
		return false;
	}

	for(int i = 0, x = 0, y = 0; i < length; i++)
	{
		if ((data[i] == '\r') || (data[i] == '\n'))
		{
			x = 0;
			y++;

			if (((i + 1) < length) && ((data[i + 1] == '\r') || (data[i + 1] == '\n')))
				i++;
		}
		else
		{
			if (Tile* tile = map->At(x, y))
			{
				tile->kind = Tile::kEmpty;

				switch(data[i])
				{
					case '.': tile->kind = Tile::kEmpty; break;
					case ' ': tile->kind = Tile::kWall; break;
					case '#': tile->kind = Tile::kWall; break;
					case 'C': tile->kind = Tile::kCheckPoint; break;
					case 'K': tile->kind = Tile::kKitten; map->_totalKittens++; break;
					case '^': tile->kind = Tile::kSpikesUp; break;
					case 'v': tile->kind = Tile::kSpikesDown; break;
					case '<': tile->kind = Tile::kSpikesLeft; break;
					case '>': tile->kind = Tile::kSpikesRight; break;
					case 'D': tile->kind = Tile::kDownOnly; break;
					case 'P': map->_playerSpawnX = x; map->_playerSpawnY = y; break;
				}
			}

			x++;
		}
	}

	delete [] data;

	return true;
}

struct Player
{
	Vector2 pos;
	Vector2 vel;
	int spawnX, spawnY;
};

Map gMap;
Player gPlayer;

void GameInit()
{
	if (!LoadMap(&gMap, "data/map.txt"))
	{
		DebugLn("Failed to load map");
	}
	else
	{
		gPlayer.spawnX = gMap._playerSpawnX;
		gPlayer.spawnY = gMap._playerSpawnY;

		gPlayer.pos = Vector2(gPlayer.spawnX * 16.0f + 8.0f, gPlayer.spawnY * 16.0f + 15.0f);
	}

	InitParticleSystem(&gParticles, 1000);
}

void GameUpdate()
{
	static bool dir;
	static bool jump;
	static bool start;
	static bool onGround;
	static int groundTime;
	static int jumpTime;
	static bool dive;
	static bool inAir;
	static bool jumpLatch;
	static bool jumpDouble;
	static int recentOnGround;
	static int wallL;
	static int wallR;
	static int lastFrame;
	static int dead;
	static int carrying;
	static int checkCarrying;
	static int carryX[2048];
	static int carryY[2048];
	static int carryHp[2048];
	static bool titleScreen;

	const int kMaxHistory = 4 * 1024;

	static Vector2 history[kMaxHistory];
	static bool historyDir[kMaxHistory];
	static bool historyOnGround[kMaxHistory];
	static bool historyJump[kMaxHistory];
	static bool historyMoving[kMaxHistory];
	static int historyPos = 0;

	if (!titleScreen)
	{
		static int flash = 0;
		flash++;

		if (gKeyFire)
		{
			titleScreen = true;
			jumpLatch = true;
		}

		DrawString(Vector2(0.0f, (kWinHeight * -0.4f) - 4.0f), kTextCentre, Colour(0.75f, 0.6f, 0.3f, 1.0f), "Super Conga Kat");

		DrawString(Vector2(0.0f, (kWinHeight * -0.325f) - 4.0f), kTextCentre, Colour(0.75f, 0.6f, 0.3f, 0.8f), "Leave no kitten behind!");

		DrawString(Vector2(0.0f, (kWinHeight * -0.15f) - 4.0f), kTextCentre, Colour(0.3f, 0.6f, 0.7f, 0.75f), "Move: Left/Right arrow keys or A/D");
		DrawString(Vector2(0.0f, (kWinHeight * -0.075f) - 4.0f), kTextCentre, Colour(0.3f, 0.6f, 0.7f, 0.75f), "Jump: Up arrow key or SPACE");

		DrawString(Vector2(0.0f, (kWinHeight * 0.075f) - 4.0f), kTextCentre, Colour(0.3f, 0.6f, 0.7f, 0.75f), "Created for Ludum Dare 24");
		DrawString(Vector2(0.0f, (kWinHeight * 0.15f) - 4.0f), kTextCentre, Colour(0.3f, 0.6f, 0.7f, 0.75f), "by Stephen Cakebread");
		DrawString(Vector2(0.0f, (kWinHeight * 0.225f) - 4.0f), kTextCentre, Colour(0.3f, 0.6f, 0.7f, 0.75f), "t: @quantumrain");

		if (((flash >> 4) & 3) < 3)
			DrawString(Vector2(0.0f, (kWinHeight * 0.4f) - 4.0f), kTextCentre, Colour(0.7f, 0.7f, 0.7f, 1.0f), "Press SPACE to start");


		return;
	}

	if (dead > 0)
	{
		if (--dead <= 0)
		{
			gPlayer.pos = Vector2(gPlayer.spawnX * 16.0f + 8.0f, gPlayer.spawnY * 16.0f + 15.0f);
			gPlayer.vel = Vector2();

			for(int i = 0; i < checkCarrying; i++)
				carryHp[i] = 0;

			for(int i = checkCarrying; i < carrying; i++)
			{
				if (Tile* tile = gMap.At(carryX[i], carryY[i]))
				{
					tile->kind = Tile::kKitten;
				}
			}

			dir = false;
			jump = false;
			onGround = true;
			groundTime = 0;
			jumpTime = 0;
			dive = false;
			inAir = false;
			jumpLatch = true;
			jumpDouble = false;
			wallL = 0;
			wallR = 0;
			carrying = checkCarrying;
			historyPos = 0;
		}
	}
	else
	{
		float moveSpeed = onGround ? 30.0f : 5.0f;

		gPlayer.vel.x -= gKeyLeft * moveSpeed;
		gPlayer.vel.x += gKeyRight * moveSpeed;
		
		bool jumpedThisFrame = false;

		if (onGround)
			jumpDouble = false;

		if ((gKeyUp || gKeyFire) && !gKeyDown && !dive)
		{
			bool grounded = (groundTime > 3) || (recentOnGround > 0);
			bool canJump = grounded;
			float boost = 0.0f;
			int wallJump = 0;

			if (!onGround && (gKeyLeft ^ gKeyRight) && (wallL > 0) && !jumpLatch)
			{
				canJump = true;
				boost = 120.0f;
				wallJump = -1;
			}

			if (!onGround && (gKeyLeft ^ gKeyRight) && (wallR > 0) && !jumpLatch)
			{
				canJump = true;
				boost = -120.0f;
				wallJump = 1;
			}

			if (!jumpDouble && !jumpLatch && !grounded && (gPlayer.vel.y > -20.0f))
				canJump = true;

			if (canJump)
			{
				if (!jump)
				{
					SoundPlay(kSid_Jump, 1.0f, 0.5f);
					SpawnJumpPuff(gPlayer.pos, wallJump);

					if (!onGround)
						gPlayer.vel.y = 0;

					if (grounded || wallJump)
						jumpDouble = false;
					else if (!jumpDouble)
						jumpDouble = true;

					gPlayer.vel.x += boost;
					gPlayer.vel.y -= 90.0f;
					jumpTime = 15;
					jump = true;
					jumpLatch = true;
					wallL = 0;
					wallR = 0;

					jumpedThisFrame = true;
				}
			}
			else
				jump = false;
		}
		else
		{
			jumpLatch = false;
			jump = false;
			jumpTime = 0;
		}

		if ((gKeyUp || gKeyFire) && (jumpTime > 0))
		{
			gPlayer.vel.y -= 5.0f;
			jumpTime--;
		}
		else
			jumpTime = 0;

		/*if (!onGround && gKeyDown)
		{
			if (!dive)
			{
				gPlayer.vel.y += 150.0f;
				dive = true;
			}
		}*/

		if (dive)
		{
			gPlayer.vel.y += 5.0f;

			if (onGround)
			{
				SoundPlay(kSid_Land, 1.0f, 0.75f);
				SpawnDiveCloud(gPlayer.pos);
				dive = false;
			}
		}

		if (gKeyLeft ^ gKeyRight)
			dir = gKeyLeft ? 0 : 1;

		if (onGround)
			gPlayer.vel.x *= 0.7f;
		else
			gPlayer.vel.x *= 0.925f;

		gPlayer.vel.x = Clamp(gPlayer.vel.x, -100.0f, 100.0f);
		gPlayer.vel.y = Clamp(gPlayer.vel.y + 5.0f, -300.0f, 300.0f);

		gPlayer.pos.x += gPlayer.vel.x * (1.0f / 60.0f);

		{ // X
			float px = floorf(gPlayer.pos.x + 0.5f);
			float py = floorf(gPlayer.pos.y + 0.5f);

			if (CollisionResult cr = gMap.HCollide((int)px, (int)(px - 6.0f), (int)(py - 7.0f))) { gPlayer.pos.x = cr.px + 6.5f; gPlayer.vel.x = 0.0f; }
			if (CollisionResult cr = gMap.HCollide((int)px, (int)(px + 6.0f), (int)(py - 7.0f))) { gPlayer.pos.x = cr.px - 6.5f; gPlayer.vel.x = 0.0f; }
												   
			if (CollisionResult cr = gMap.HCollide((int)px, (int)(px - 6.0f), (int)(py - 1.0f))) { gPlayer.pos.x = cr.px + 6.5f; gPlayer.vel.x = 0.0f; }
			if (CollisionResult cr = gMap.HCollide((int)px, (int)(px + 6.0f), (int)(py - 1.0f))) { gPlayer.pos.x = cr.px - 6.5f; gPlayer.vel.x = 0.0f; }
		}

		onGround = false;
		gPlayer.pos.y += gPlayer.vel.y * (1.0f / 60.0f);

		{ // Y
			float px = floorf(gPlayer.pos.x + 0.5f);
			float py = floorf(gPlayer.pos.y + 0.5f);

			if (CollisionResult cr = gMap.VCollide((int)(px - 5.0f), (int)(py - 4.0f), (int)(py - 9.0f))) { gPlayer.pos.y = cr.py + 9.5f; gPlayer.vel.y = 0.0f; jumpTime = 0; }
			if (CollisionResult cr = gMap.VCollide((int)(px - 5.0f), (int)(py - 4.0f), (int)(py + 1.0f))) { gPlayer.pos.y = cr.py - 1.5f; gPlayer.vel.y = 0.0f; onGround = true; }

			if (CollisionResult cr = gMap.VCollide((int)(px + 5.0f), (int)(py - 4.0f), (int)(py - 9.0f))) { gPlayer.pos.y = cr.py + 9.5f; gPlayer.vel.y = 0.0f; jumpTime = 0; }
			if (CollisionResult cr = gMap.VCollide((int)(px + 5.0f), (int)(py - 4.0f), (int)(py + 1.0f))) { gPlayer.pos.y = cr.py - 1.5f; gPlayer.vel.y = 0.0f; onGround = true; }
		}

		{ // Wall jump check
			float px = floorf(gPlayer.pos.x + 0.5f);
			float py = floorf(gPlayer.pos.y + 0.5f);

			wallL = Max(wallL - 1, 0);
			wallR = Max(wallR - 1, 0);

			if (CollisionResult cr = gMap.HCollide((int)px, (int)(px - 8.0f), (int)(py - 7.0f))) { wallL = 3; }
			if (CollisionResult cr = gMap.HCollide((int)px, (int)(px + 8.0f), (int)(py - 7.0f))) { wallR = 3; }

			if (CollisionResult cr = gMap.HCollide((int)px, (int)(px - 8.0f), (int)(py - 1.0f))) { wallL = 3; }
			if (CollisionResult cr = gMap.HCollide((int)px, (int)(px + 8.0f), (int)(py - 1.0f))) { wallR = 3; }
		}

		{ // Object check
			int x0 = (int)floorf((gPlayer.pos.x - 8.0f) / 16.0f);
			int y0 = (int)floorf((gPlayer.pos.y - 8.0f) / 16.0f);
			int x1 = (int)ceilf((gPlayer.pos.x + 8.0f) / 16.0f);
			int y1 = (int)ceilf((gPlayer.pos.y + 8.0f) / 16.0f);

			Vector2 playerCentre(gPlayer.pos + Vector2(0.0f, -3.0f));
			Vector2 playerSize(8.0f, 4.0f);

			for(int y = y0; y <= y1; y++)
			{
				for(int x = x0; x <= x1; x++)
				{
					if (Tile* tile = gMap.At(x, y))
					{
						switch(tile->kind)
						{
							case Tile::kCheckPoint:
							{
								Vector2 tileCentre((x * 16.0f) + 8.0f, (y * 16.0f) + 7.0f);
								Vector2 tileSize(12.0f, 18.0f);

								if (OverlapsRect(playerCentre, playerSize, tileCentre, tileSize))
								{
									if ((gPlayer.spawnX != x) || (gPlayer.spawnY != y) || (checkCarrying != carrying))
									{
										SoundPlay(kSid_CheckPoint, 1.0f, 1.0f);
									}

									gPlayer.spawnX = x;
									gPlayer.spawnY = y;

									checkCarrying = carrying;
								}
							}
							break;

							case Tile::kKitten:
							{
								Vector2 tileCentre((x * 16.0f) + 8.0f, (y * 16.0f) + 13.0f);
								Vector2 tileSize(8.0f, 6.0f);

								if (OverlapsRect(playerCentre, playerSize, tileCentre, tileSize))
								{
									tile->kind = Tile::kEmpty;
									carryX[carrying] = x;
									carryY[carrying] = y;
									carryHp[carrying] = historyPos;
									carrying++;

									SoundPlay(kSid_Kitten, 1.0f, 1.0f);
								}
							}
							break;

							case Tile::kSpikesUp:
							case Tile::kSpikesDown:
							case Tile::kSpikesLeft:
							case Tile::kSpikesRight:
							{
								Vector2 tileCentre;
								Vector2 tileSize;

								if (tile->kind == Tile::kSpikesUp)			{ tileCentre = Vector2((x * 16.0f) + 8.0f, (y * 16.0f) + 14.0f);	tileSize = Vector2(14.0f, 4.0f); }
								else if (tile->kind == Tile::kSpikesDown)	{ tileCentre = Vector2((x * 16.0f) + 8.0f, (y * 16.0f) + 2.0f);		tileSize = Vector2(14.0f, 4.0f); }
								else if (tile->kind == Tile::kSpikesLeft)	{ tileCentre = Vector2((x * 16.0f) + 14.0f, (y * 16.0f) + 8.0f);	tileSize = Vector2(4.0f, 14.0f); }
								else if (tile->kind == Tile::kSpikesRight)	{ tileCentre = Vector2((x * 16.0f) + 2.0f, (y * 16.0f) + 8.0f);		tileSize = Vector2(4.0f, 14.0f); }

								if (OverlapsRect(playerCentre, playerSize, tileCentre, tileSize))
								{
									dead = 20;
									SoundPlay(kSid_Dead, 1.0f, 1.0f);
								}
							}
							break;
						}
					}
				}
			}
		}

		if (onGround)
		{
			groundTime++;
			recentOnGround = 8;
		}
		else
		{
			groundTime = 0;
			recentOnGround = Max(recentOnGround - 1, 0);
		}

		if (onGround)
		{
			if (inAir)
			{
				SoundPlay(kSid_Jump, 0.5f, 0.3f);
				SpawnLandCloud(gPlayer.pos);
				inAir = false;
			}
		}
		else
			inAir = true;

		int hp = historyPos % kMaxHistory;
		history[hp] = gPlayer.pos;
		historyDir[hp] = dir;
		historyOnGround[hp] = onGround;
		historyJump[hp] = jumpedThisFrame;
		historyMoving[hp] = gKeyLeft ^ gKeyRight;
		historyPos++;
	}

	// Player sprite

	static int anim =0;
	anim++;

	int frame = 0;

	if (gKeyLeft ^ gKeyRight)
		frame = 4 + ((anim >> 3) % 3);
	else
		frame = ((anim >> 5) % 4);

	if (!onGround)
	{
		if (dive)
			frame = 10;
		else if (fabsf(gPlayer.vel.y) < 30.0f)
			frame = 8;
		else
			frame = (gPlayer.vel.y < 0.0f) ? 7 : 9;
	}

	if (onGround && (gKeyLeft ^ gKeyRight))
	{
		if ((anim % 24) == 16)
		{
			Vector2 pos = gPlayer.pos + Vector2((dir ? -6.0f : 6.0f), 1.0f);

			SpawnParticle(&gParticles, pos, RandBox(20.0f, 5.0f) + Vector2(0.0f, -15.0f), 0.2f, Colour(0.5f, 1.0f), Colour(0.5f, 1.0f), 1.0f, 1.0f);
			SpawnParticle(&gParticles, pos, RandBox(20.0f, 5.0f) + Vector2(0.0f, -15.0f), 0.2f, Colour(0.5f, 1.0f), Colour(0.5f, 1.0f), 1.0f, 1.0f);

			SoundPlay(kSid_Jump, 3.0f, 0.1f);
		}
	}

	if (dead > 0)
	{
		if ((dead & 3) < 2)
			frame = lastFrame + 32;
		else
			frame = - 1;
	}
	else
	{
		lastFrame = frame;
	}

	// Render map

	Vector2 pan = -gPlayer.pos;

	{
		int x0 = (int)floorf((gPlayer.pos.x - kWinWidth * 0.5f) / 16.0f);
		int y0 = (int)floorf((gPlayer.pos.y - kWinHeight * 0.5f) / 16.0f);
		int x1 = (int)ceilf((gPlayer.pos.x + kWinWidth * 0.5f) / 16.0f);
		int y1 = (int)ceilf((gPlayer.pos.y + kWinHeight * 0.5f) / 16.0f);

		for(int y = y0; y <= y1; y++)
		{
			for(int x = x0; x <= x1; x++)
			{
				Tile::Kind t = gMap.AtKind(x, y);

				if (t == Tile::kEmpty)
					continue;

				Vector2 p0(x * 16.0f, y * 16.0f);
				Vector2 p1((x + 1) * 16.0f, (y + 1) * 16.0f);

				int tile = -1;

				switch(t)
				{
					case Tile::kWall:
						if (gMap.AtKind(x, y - 1) != Tile::kWall)
							tile = 18;
						else
							tile = 16;
					break;

					case Tile::kCheckPoint:
						tile = (gPlayer.spawnX == x) && (gPlayer.spawnY == y) ? 21 : 20;
					break;

					case Tile::kKitten:
						if (gMap.AtKind(x, y + 1) == Tile::kWall)
							tile = 50;
						else
							tile = 52;
					break;

					case Tile::kSpikesUp: tile = 19; break;
					case Tile::kSpikesDown: tile = 22; break;
					case Tile::kSpikesLeft: tile = 24; break;
					case Tile::kSpikesRight: tile = 23; break;
					case Tile::kDownOnly: tile = 25; break;
				}

				if (tile >= 0)
					DrawRect(p0 + pan, p1 + pan, tile, 0, Colour());
			}
		}
	}

	if (frame >= 0)
	{
		int kittenLanded = -1;
		int kittenJump = -1;

		for(int i = carrying - 1; i >= 0; i--)
		{
			int hp = Max(historyPos - ((i * 8) + 10), 0);
			int hpm = hp % kMaxHistory;

			Vector2 pos = history[hpm];
			bool hd = historyDir[hpm];

			int sprite = 48;

			if ((hp > 0) && (historyOnGround[hpm] ^ historyOnGround[(hpm + kMaxHistory - 1) % kMaxHistory]))
				kittenLanded = i;

			if (historyJump[hpm])
				kittenJump = i;

			if (!historyOnGround[hpm] || (historyMoving[hpm] && ((hp >> 3) & 1)))
				sprite = 49;

			if (carryHp[i] > hp)
			{
				pos = Vector2(carryX[i] * 16.0f + 8.0f, carryY[i] * 16.0f + 15.0f);
				sprite = ((hp & 3) < 2) ? 50 : 51;
				hd = false;
			}

			DrawSprite(pos + pan - Vector2(hd ? 1.0f : -1.0f, 7.0f), Vector2(1.0f), sprite, hd ? kFlipX : 0, Colour());
		}

		if (kittenJump >= 0)
			SoundPlay(kSid_KittenJump, 1.0f, Clamp(0.1f - kittenLanded * 0.25f, 0.1f, 0.02f));
		else if (kittenLanded >= 0)
			SoundPlay(kSid_KittenJump, 0.5f, Clamp(0.1f - kittenLanded * 0.25f, 0.1f, 0.02f));

		DrawSprite(gPlayer.pos + pan - Vector2(dir ? 1.0f : -1.0f, 7.0f), Vector2(1.0f), frame, dir ? kFlipX : 0, Colour());
	}

	UpdateParticleSystem(&gParticles, pan);

	DrawString(Vector2(-(kWinWidth * 0.5f) + 2, (kWinHeight * 0.5f) - 9), kTextLeft, Colour(), (carrying== gMap._totalKittens) ? "Kittens: %i / %i - YOU WIN!" : "Kittens: %i / %i", carrying, gMap._totalKittens);
}
