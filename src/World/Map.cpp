#include "Map.h"

// ---------------------------------------------------------------------------
// Map globals
// Defined here; declared extern in Map.h so every TU that includes Map.h
// can access them without multiple-definition errors.
// ---------------------------------------------------------------------------
int MAP_W = 1;
int MAP_H = 1;
std::vector<std::vector<TileType>> MAP;

void resizeMap(int w, int h)
{
	MAP_W = w;
	MAP_H = h;
	MAP.assign(h, std::vector<TileType>(w, TileType::Empty));
}