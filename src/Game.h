#ifndef GAME_H
#define GAME_H

const int MAX_WORM_BLOCKS = 16;
const int MAX_WORMS = 6;

enum tile {
	TILE_WALL,
	TILE_EMPTY
};

struct ivec2 {
	int x, y;

	ivec2(int x_ = 0, int y_ = 0) : x(x_), y(y_) { }
};

struct map {
	ivec2 size;
	uint8_t* data;
	colour colours[MAX_WORMS];
	ivec2 targets[MAX_WORMS];

	map() : data() { }
	~map() { destroy(); }

	void destroy();

	tile at(int x, int y) {
		if (x < 0 || y < 0 || x >= size.x || y >= size.y)
			return TILE_WALL;
		return (tile)data[y * size.x + x];
	}
};

struct worm_block {
	ivec2 pos;
	int age;

	worm_block() : age() { }
};

struct worm {
	int num_blocks;
	int active_block;
	int age;
	worm_block blocks[MAX_WORM_BLOCKS];

	worm() : num_blocks(), active_block(), age() { }
};

struct game_state {
	int num_worms;
	int active_worm;
	worm worms[MAX_WORMS];

	game_state() : num_worms(), active_worm() { }

	void reset();
};

vec2 to_screen(ivec2 v);
bool load_map(map* m, game_state* gs, const char* path);
void render_map(map* m, game_state* gs, vec2 scale);

#endif // GAME_H