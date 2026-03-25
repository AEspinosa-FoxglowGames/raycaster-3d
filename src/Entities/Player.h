#pragma once
#include <glm/glm.hpp>
#include "Inventory.h"
#include "Journal.h"

struct Player {
	glm::vec2 pos = { 2.5f, 2.5f };
	float     angle = -1.5707963f;  // -PI/2, facing north

	int   health = 100;
	int   maxHealth = 100;

	float stamina = 100.0f;
	float maxStamina = 100.0f;

	float speed = 3.5f;
	float sprintSpeed = 6.0f;

	bool running = false;
	bool exhausted = false;

	float pitch = 0.0f;
	float pitchSpeed = 200.0f;
	float pitchMax = 400.0f;

	// ---- Flashlight state ----
	// Toggled with F.  Only active when Flashlight is in inventory.
	// flashlightFailAccum accumulates time; once per second a random
	// check is made to see if the flashlight cuts out.
	bool  flashlightOn = false;
	float flashlightFailAccum = 0.0f;

	// Persistent across zone transitions (never cleared by ZoneManager)
	Inventory inventory;
	Journal   journal;
};
