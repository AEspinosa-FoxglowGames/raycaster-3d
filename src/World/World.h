#pragma once
#include <vector>
#include "Entity.h"
#include "Map.h"

// Global entity list.  World.cpp owns the definitions.
extern std::vector<Entity> entities;

// Sanity-check entity positions against the map at startup.
void validateEntities(const std::vector<Entity>& entities);