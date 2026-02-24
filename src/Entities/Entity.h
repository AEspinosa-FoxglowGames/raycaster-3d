// Entity.h
#pragma once
#include <glm/glm.hpp>

enum class EntityType {
	Note,
	Torch,
	// add more later
};

struct Entity {
	glm::vec2 pos;
	EntityType type;
};