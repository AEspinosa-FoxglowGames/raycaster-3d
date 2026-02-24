#include "World.h"
#include "Map.h"
#include <iostream>

std::vector<Entity> entities = {
	{ {3.0f, 3.0f}, EntityType::Note },
};

void validateEntities(const std::vector<Entity>& entities)
{
	for (const auto& e : entities) {
		int x = (int)e.pos.x;
		int y = (int)e.pos.y;
		if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
			std::cerr << "Entity out of bounds at " << e.pos.x << ", " << e.pos.y << "\n";
			continue;
		}
		if (MAP[y][x] == 1)
			std::cerr << "Entity inside wall at " << e.pos.x << ", " << e.pos.y << "\n";
	}
}