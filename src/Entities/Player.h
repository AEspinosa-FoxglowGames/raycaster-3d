#pragma once
#include <glm/glm.hpp>

struct Player {
	glm::vec2 pos = { 2.5f, 2.5f }; // starting position in map space
	float angle = 0.0f;            // direction in radians
	float speed = 2.0f;
	float sprintSpeed = 4.0f;
	float turnSpeed = 2.0f;
	int health = 100;
	int maxHealth = 100;
	float stamina = 100;
	float maxStamina = 100;
	bool running = false;
	bool exhausted = false;
};