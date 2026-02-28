#pragma once
#include <glm/glm.hpp>
#include <cmath>
#include "Map.h"

struct RayHit {
	float    dist;
	bool     hitNS;      // true = N/S face (darker); false = E/W face (lighter)
	float    hitPt;      // [0,1] texture U along the face
	int      wallX;
	int      wallY;
	TileType tile;
	bool     isDoorHit;  // true when this hit is the recessed door plane
};

// Door geometry within one tile (U axis across the hit face):
//   [frame][  panel  ][frame]
//        DOOR_MIN   DOOR_MAX
//
// Key stability trick:
// - We compute U (hitPt) from the *boundary* intersection, not from the recessed plane.
//   U should not change with depth, and this avoids angle-dependent "ghost" slices.
// - We only change the returned distance to push the door slightly back (DOOR_DEPTH).

constexpr float DOOR_MIN = 0.15f;
constexpr float DOOR_MAX = 0.85f;
constexpr float DOOR_DEPTH = 0.09f;   // small recess; keep < ~0.1 to avoid leaving the door cell
constexpr float DOOR_EPS = 1e-4f;   // seam stability

inline RayHit castRay(glm::vec2 pos, float angle)
{
	glm::vec2 dir = { cosf(angle), sinf(angle) };

	int mapX = (int)pos.x;
	int mapY = (int)pos.y;

	float deltaDistX = (dir.x == 0.0f) ? 1e30f : fabsf(1.0f / dir.x);
	float deltaDistY = (dir.y == 0.0f) ? 1e30f : fabsf(1.0f / dir.y);

	int   stepX, stepY;
	float sideDistX, sideDistY;

	if (dir.x < 0.0f) { stepX = -1; sideDistX = (pos.x - mapX) * deltaDistX; }
	else { stepX = 1; sideDistX = (mapX + 1.0f - pos.x) * deltaDistX; }

	if (dir.y < 0.0f) { stepY = -1; sideDistY = (pos.y - mapY) * deltaDistY; }
	else { stepY = 1; sideDistY = (mapY + 1.0f - pos.y) * deltaDistY; }

	bool hitNS = false;

	for (int i = 0; i < 64; ++i)
	{
		// Step DDA
		if (sideDistX < sideDistY) {
			sideDistX += deltaDistX; mapX += stepX; hitNS = false;
		}
		else {
			sideDistY += deltaDistY; mapY += stepY; hitNS = true;
		}

		TileType t = getTile(mapX, mapY);

		// Door special-case
		if (isDoor(t))
		{
			// Only render the approached front face. Side-entry behaves like solid wall.
			if (!isDoorFrontFace(t, hitNS)) goto solidWall;

			// Distance to boundary face we just crossed.
			float distToBoundary = hitNS
				? (mapY - pos.y + (1 - stepY) / 2.0f) / dir.y
				: (mapX - pos.x + (1 - stepX) / 2.0f) / dir.x;

			// Compute U from the boundary intersection (stable; doesn't depend on recess depth)
			float wallHit = hitNS ? (pos.x + distToBoundary * dir.x) : (pos.y + distToBoundary * dir.y);
			float u = fmodf(wallHit, 1.0f);
			if (u < 0.0f) u += 1.0f;

			// Bias away from exact seam values so we don't flicker between panel/frame
			if (fabsf(u - DOOR_MIN) < DOOR_EPS) u = DOOR_MIN + DOOR_EPS;
			if (fabsf(u - DOOR_MAX) < DOOR_EPS) u = DOOR_MAX - DOOR_EPS;

			return { fabsf(distToBoundary) + DOOR_DEPTH, hitNS, u, mapX, mapY, t, true };
		}

	solidWall:
		if (isOpaque(t))
		{
			float dist = hitNS
				? (mapY - pos.y + (1 - stepY) / 2.0f) / dir.y
				: (mapX - pos.x + (1 - stepX) / 2.0f) / dir.x;

			float wallHit = hitNS ? pos.x + dist * dir.x : pos.y + dist * dir.y;
			float hitPt = fmodf(wallHit, 1.0f);
			if (hitPt < 0.0f) hitPt += 1.0f;
			return { fabsf(dist), hitNS, hitPt, mapX, mapY, t, false };
		}
	}

	return { 64.0f, false, 0.0f, mapX, mapY, TileType::WallStone, false };
}
