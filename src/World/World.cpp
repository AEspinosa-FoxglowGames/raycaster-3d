#include "World.h"
#include "Map.h"
#include <iostream>

using T = TileType;

// ---------------------------------------------------------------------------
// Map definition
// Edited here; getTile/setTile used everywhere else.
// ---------------------------------------------------------------------------
std::array<std::array<TileType, MAP_W>, MAP_H> MAP = { {
		// Row 0 – top border
		{T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::WallBrick,T::WallBrick,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallBrick,T::WallBrick,T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::WallBrick,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallBrick,T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone,T::WallStone,T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::WallBrick,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallBrick,T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::WallBrick,T::WallBrick,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallBrick,T::WallBrick,T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		// Row 15-16 – hallway junction, single door
		// DoorClosedH = door in a horizontal wall, player walks N/S through it (hitNS=true)
		{T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::DoorClosedH,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone},
		{T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::Empty,    T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		{T::WallStone,T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::Empty,    T::WallStone},
		// Row 31 – bottom border
		{T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone,T::WallStone},
	} };

// ---------------------------------------------------------------------------
// Entity list
// ---------------------------------------------------------------------------
std::vector<Entity> entities = {
	// Wall-face note: wallCell tells the renderer which wall face to decal.
	{.pos = {3.0f, 3.0f}, .type = EntityType::Note, .wallCell = {3, 3}, .noteText = "Something was here." },
};

// ---------------------------------------------------------------------------
void validateEntities(const std::vector<Entity>& ents)
{
	for (const auto& e : ents) {
		int x = (int)e.pos.x;
		int y = (int)e.pos.y;
		if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
			std::cerr << "[World] Entity out of bounds at ("
				<< e.pos.x << ", " << e.pos.y << ")\n";
			continue;
		}
		if (isSolid(getTile(x, y)))
			std::cerr << "[World] Entity inside solid tile at ("
			<< e.pos.x << ", " << e.pos.y << ")\n";
	}
}