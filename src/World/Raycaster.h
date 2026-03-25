#pragma once
#include <glm/glm.hpp>
#include <cmath>
#include "Map.h"

struct RayHit {
	float    dist;
	bool     hitNS;
	float    hitPt;
	int      wallX;
	int      wallY;
	TileType tile;
	bool     isDoorHit;

	// Populated when primary hit is glass — the opaque wall seen through it.
	bool     hasSecondary = false;
	float    secondaryDist = 0.0f;
	bool     secondaryNS = false;
	float    secondaryHitPt = 0.0f;
	int      secondaryWallX = 0;
	int      secondaryWallY = 0;
	TileType secondaryTile = TileType::WallStone;
};

constexpr float DOOR_DEPTH = 0.02f;

// ---------------------------------------------------------------------------
// Open door panel intersection
//
// DoorOpenH: panel rests against the WEST edge of the cell (x = mapX).
//   This is an E/W face → hitNS = false, brighter shading.
//   UV along the panel = Y fraction within the cell.
//
// DoorOpenV: panel rests against the NORTH edge (y = mapY).
//   This is a N/S face → hitNS = true, darker shading.
//   UV along the panel = X fraction within the cell.
//
// We only accept intersections in front of the ray (t > eps) and within the
// cell bounds, so the panel doesn't bleed into adjacent cells.
// ---------------------------------------------------------------------------
inline bool intersectOpenDoor(glm::vec2 pos, glm::vec2 dir,
	int mapX, int mapY, TileType t,
	RayHit& out)
{
	constexpr float EPS = 1e-4f;

	if (t == TileType::DoorOpenH)
	{
		// Panel at x = mapX (west edge), E/W face
		if (fabsf(dir.x) < EPS) return false;
		float tHit = (mapX - pos.x) / dir.x;
		if (tHit < EPS) return false;
		float hitY = pos.y + tHit * dir.y;
		float u = hitY - (float)mapY;           // fraction along Y in this cell
		if (u < 0.0f || u > 1.0f) return false;
		out = { tHit, false, u, mapX, mapY, TileType::DoorClosedH, true };
		return true;
	}

	if (t == TileType::DoorOpenV)
	{
		// Panel at y = mapY (north edge), N/S face
		if (fabsf(dir.y) < EPS) return false;
		float tHit = (mapY - pos.y) / dir.y;
		if (tHit < EPS) return false;
		float hitX = pos.x + tHit * dir.x;
		float u = hitX - (float)mapX;
		if (u < 0.0f || u > 1.0f) return false;
		out = { tHit, true, u, mapX, mapY, TileType::DoorClosedV, true };
		return true;
	}

	return false;
}
inline bool hasLOS(glm::vec2 from, glm::vec2 to)
{
	glm::vec2 dir = to - from;
	float dist = sqrtf(dir.x * dir.x + dir.y * dir.y);
	if (dist < 0.0001f) return true;
	dir.x /= dist; dir.y /= dist;

	int mapX = (int)from.x;
	int mapY = (int)from.y;
	int endX = (int)to.x;
	int endY = (int)to.y;

	float deltaDistX = (dir.x == 0.0f) ? 1e30f : fabsf(1.0f / dir.x);
	float deltaDistY = (dir.y == 0.0f) ? 1e30f : fabsf(1.0f / dir.y);

	int stepX, stepY;
	float sideDistX, sideDistY;

	if (dir.x < 0.0f) { stepX = -1; sideDistX = (from.x - mapX) * deltaDistX; }
	else { stepX = 1; sideDistX = (mapX + 1.0f - from.x) * deltaDistX; }
	if (dir.y < 0.0f) { stepY = -1; sideDistY = (from.y - mapY) * deltaDistY; }
	else { stepY = 1; sideDistY = (mapY + 1.0f - from.y) * deltaDistY; }

	for (int i = 0; i < 32; ++i)
	{
		if (mapX == endX && mapY == endY) return true;
		if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; }
		else { sideDistY += deltaDistY; mapY += stepY; }
		TileType t = getTile(mapX, mapY);
		if (isOpaque(t)) return false;
	}
	return true;
}
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
	RayHit glassHit{};   // saved if we pass through glass
	bool   sawGlass = false;

	for (int i = 0; i < 64; ++i)
	{
		if (sideDistX < sideDistY) {
			sideDistX += deltaDistX; mapX += stepX; hitNS = false;
		}
		else {
			sideDistY += deltaDistY; mapY += stepY; hitNS = true;
		}

		TileType t = getTile(mapX, mapY);

		// --- Glass: record primary hit, keep marching for the wall behind ---
		if (isGlass(t) && !sawGlass)
		{
			float dist = hitNS
				? (mapY - pos.y + (1 - stepY) / 2.0f) / dir.y
				: (mapX - pos.x + (1 - stepX) / 2.0f) / dir.x;
			float wallHit = hitNS ? pos.x + dist * dir.x : pos.y + dist * dir.y;
			float hitPt = fmodf(wallHit, 1.0f);
			if (hitPt < 0.0f) hitPt += 1.0f;

			glassHit = { fabsf(dist), hitNS, hitPt, mapX, mapY, t, false };
			sawGlass = true;
			continue;   // keep marching — find wall behind
		}

		// --- Closed door ---
		if (isDoor(t))
		{
			if (!isDoorFrontFace(t, hitNS)) goto solidWall;

			float distToBoundary = hitNS
				? (mapY - pos.y + (1 - stepY) / 2.0f) / dir.y
				: (mapX - pos.x + (1 - stepX) / 2.0f) / dir.x;

			float wallHit = hitNS
				? (pos.x + distToBoundary * dir.x)
				: (pos.y + distToBoundary * dir.y);
			float u = fmodf(wallHit, 1.0f);
			if (u < 0.0f) u += 1.0f;

			RayHit result = { fabsf(distToBoundary) + DOOR_DEPTH, hitNS, u, mapX, mapY, t, true };
			if (sawGlass) {
				// Door seen through glass — attach glass as secondary info.
				// Swap: glass is the "front", door is "behind".
				result.hasSecondary = true;
				result.secondaryDist = result.dist;
				result.secondaryNS = result.hitNS;
				result.secondaryHitPt = result.hitPt;
				result.secondaryWallX = result.wallX;
				result.secondaryWallY = result.wallY;
				result.secondaryTile = result.tile;
				result.dist = glassHit.dist;
				result.hitNS = glassHit.hitNS;
				result.hitPt = glassHit.hitPt;
				result.wallX = glassHit.wallX;
				result.wallY = glassHit.wallY;
				result.tile = glassHit.tile;
				result.isDoorHit = false;
			}
			return result;
		}

		// --- Open door: check sub-tile panel intersection ---
		if (isOpenDoor(t))
		{
			RayHit panelHit;
			if (intersectOpenDoor(pos, dir, mapX, mapY, t, panelHit))
			{
				if (sawGlass) {
					panelHit.hasSecondary = true;
					panelHit.secondaryDist = panelHit.dist;
					panelHit.secondaryNS = panelHit.hitNS;
					panelHit.secondaryHitPt = panelHit.hitPt;
					panelHit.secondaryWallX = panelHit.wallX;
					panelHit.secondaryWallY = panelHit.wallY;
					panelHit.secondaryTile = panelHit.tile;
					panelHit.dist = glassHit.dist;
					panelHit.hitNS = glassHit.hitNS;
					panelHit.hitPt = glassHit.hitPt;
					panelHit.wallX = glassHit.wallX;
					panelHit.wallY = glassHit.wallY;
					panelHit.tile = glassHit.tile;
					panelHit.isDoorHit = false;
				}
				return panelHit;
			}
			continue;
		}

	solidWall:
		if (isSolid(t))
		{
			float dist = hitNS
				? (mapY - pos.y + (1 - stepY) / 2.0f) / dir.y
				: (mapX - pos.x + (1 - stepX) / 2.0f) / dir.x;
			float wallHit = hitNS ? pos.x + dist * dir.x : pos.y + dist * dir.y;
			float hitPt = fmodf(wallHit, 1.0f);
			if (hitPt < 0.0f) hitPt += 1.0f;

			RayHit result = { fabsf(dist), hitNS, hitPt, mapX, mapY, t, false };
			if (sawGlass) {
				// This is the opaque wall behind the glass.
				// Primary = glass (front), secondary = this wall (behind).
				result.hasSecondary = true;
				result.secondaryDist = result.dist;
				result.secondaryNS = result.hitNS;
				result.secondaryHitPt = result.hitPt;
				result.secondaryWallX = result.wallX;
				result.secondaryWallY = result.wallY;
				result.secondaryTile = result.tile;
				result.dist = glassHit.dist;
				result.hitNS = glassHit.hitNS;
				result.hitPt = glassHit.hitPt;
				result.wallX = glassHit.wallX;
				result.wallY = glassHit.wallY;
				result.tile = glassHit.tile;
				result.isDoorHit = false;
			}
			return result;
		}
	}

	// If we only ever hit glass and nothing behind it, return the glass hit
	if (sawGlass) return glassHit;

	return { 64.0f, false, 0.0f, mapX, mapY, TileType::WallStone, false };
}