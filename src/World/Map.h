#pragma once
#include <array>
#include <cstdint>

// ---------------------------------------------------------------------------
// Tile type ranges
//
//   0        : Empty / floor
//   1 – 49   : Walls  (solid, opaque)
//   50 – 59  : Doors, closed/locked  (solid)
//   60 – 69  : Doors, open  (passable)
//   90 – 99  : Zone triggers  (passable)
//
// Door orientation:
//   DoorClosedH (50) = door face is horizontal (player walks N/S through it)
//                      → raycaster hits it with hitNS=true
//   DoorClosedV (51) = door face is vertical   (player walks E/W through it)
//                      → raycaster hits it with hitNS=false
//   DoorFrame   (52) = synthetic, never placed in map. Emitted by raycaster
//                      for the wall-stub sides of a half-width door.
//   DoorLocked  (53) = same as DoorClosedH but locked (no key yet)
// ---------------------------------------------------------------------------

enum class TileType : uint8_t {
	Empty = 0,

	// Walls
	WallStone = 1,
	WallBrick = 2,
	WallConcrete = 3,
	WallSealed = 4,

	// Closed doors  (solid)
	DoorClosedH = 50,  // approached from north or south (hitNS=true)
	DoorClosedV = 51,  // approached from east  or west  (hitNS=false)
	DoorFrame = 52,  // synthetic -- frame side strips
	DoorLocked = 53,  // hitNS=true locked variant

	// Open doors  (passable, transparent)
	DoorOpen = 60,

	// Zone triggers
	ZoneExit = 90,
	ZoneTransition = 91,
};

inline bool isSolid(TileType t) {
	auto v = static_cast<uint8_t>(t);
	return v >= 1 && v <= 59;
}

inline bool isOpaque(TileType t) {
	return isSolid(t);
}

// True for any closed door that uses half-width midpoint rendering
inline bool isDoor(TileType t) {
	return t == TileType::DoorClosedH
		|| t == TileType::DoorClosedV
		|| t == TileType::DoorLocked;
}

// Returns true if the ray entered the door from the correct (front) face.
// Side-entering rays should render a plain wall instead.
inline bool isDoorFrontFace(TileType t, bool hitNS) {
	if (t == TileType::DoorClosedH || t == TileType::DoorLocked)
		return hitNS;   // front face is N or S
	if (t == TileType::DoorClosedV)
		return !hitNS;  // front face is E or W
	return false;
}

// ---------------------------------------------------------------------------
// Map data
// ---------------------------------------------------------------------------

constexpr int MAP_W = 16;
constexpr int MAP_H = 32;

extern std::array<std::array<TileType, MAP_W>, MAP_H> MAP;

inline TileType getTile(int x, int y) {
	if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return TileType::WallStone;
	return MAP[y][x];
}

inline void setTile(int x, int y, TileType t) {
	if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return;
	MAP[y][x] = t;
}