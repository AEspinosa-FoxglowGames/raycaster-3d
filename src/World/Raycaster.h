#pragma once
#include <glm/glm.hpp>
#include <cmath>
#include "Map.h"

struct RayHit {
	float dist;      // perpendicular distance to wall
	bool  hitNS;     // true = hit north/south wall, false = east/west
	float hitPt;// where along the wall face, 0.0 to 1.0
	int wallX;  // map cell that was hit
	int wallY;
};

inline RayHit castRay(glm::vec2 pos, float angle)
{
	glm::vec2 dir = { cosf(angle), sinf(angle) };

	// Which map cell we're in
	int mapX = (int)pos.x;
	int mapY = (int)pos.y;

	// How far along the ray we travel to cross one cell in X or Y
	float deltaDistX = (dir.x == 0) ? 1e30f : fabsf(1.0f / dir.x);
	float deltaDistY = (dir.y == 0) ? 1e30f : fabsf(1.0f / dir.y);

	// Step direction and initial side distances
	int stepX, stepY;
	float sideDistX, sideDistY;

	if (dir.x < 0) {
		stepX = -1;
		sideDistX = (pos.x - mapX) * deltaDistX;
	}
	else {
		stepX = 1;
		sideDistX = (mapX + 1.0f - pos.x) * deltaDistX;
	}

	if (dir.y < 0) {
		stepY = -1;
		sideDistY = (pos.y - mapY) * deltaDistY;
	}
	else {
		stepY = 1;
		sideDistY = (mapY + 1.0f - pos.y) * deltaDistY;
	}

	// DDA - step through cells until we hit a wall
	bool hitNS = false;
	for (int i = 0; i < 64; i++)
	{
		if (sideDistX < sideDistY) {
			sideDistX += deltaDistX;
			mapX += stepX;
			hitNS = false;
		}
		else {
			sideDistY += deltaDistY;
			mapY += stepY;
			hitNS = true;
		}

		if (MAP[mapY][mapX] == 1) break;
	}

	// Perpendicular distance (avoids fisheye)
	float dist = hitNS
		? (mapY - pos.y + (1 - stepY) / 2.0f) / dir.y
		: (mapX - pos.x + (1 - stepX) / 2.0f) / dir.x;
	float wallHit;
	if (hitNS)
		wallHit = pos.x + dist * dir.x;
	else
		wallHit = pos.y + dist * dir.y;

	float hitPt = fmodf(wallHit, 1.0f); // strip whole number, keep decimal
	return { fabsf(dist), hitNS, hitPt, mapX, mapY };
}