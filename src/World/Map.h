#pragma once
#include <vector>
#include <cstdint>

enum class TileType : uint8_t {
	Empty = 0,
	WallStone = 1,
	WallBrick = 2,
	WallConcrete = 3,
	WallSealed = 4,
	WallGlass = 5,      // semi-transparent: ray passes through to find wall behind
	DoorClosedH = 50,
	DoorClosedV = 51,
	DoorFrame = 52,
	DoorLocked = 53,
	DoorOpenH = 60,
	DoorOpenV = 61,
	ZoneExit = 90,   // passable, triggers zone load
	ZoneTransition = 91,   // passable, triggers zone load (alias)
};

// Runtime map dimensions -- set by ZoneManager::load()
extern int MAP_W;
extern int MAP_H;
extern std::vector<std::vector<TileType>> MAP;

void resizeMap(int w, int h);   // clears map to WallStone, sets MAP_W/MAP_H

inline TileType getTile(int x, int y) {
	if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return TileType::WallStone;
	return MAP[y][x];
}
inline void setTile(int x, int y, TileType t) {
	if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return;
	MAP[y][x] = t;
}

inline bool isSolid(TileType t) {
	auto v = static_cast<uint8_t>(t);
	return v >= 1 && v <= 59;
}
inline bool isGlass(TileType t) { return t == TileType::WallGlass; }
// Glass is solid for collision/z-buffer but transparent for rendering
inline bool isOpaque(TileType t) { return isSolid(t) && !isGlass(t); }

inline bool isDoor(TileType t) {
	return t == TileType::DoorClosedH
		|| t == TileType::DoorClosedV
		|| t == TileType::DoorLocked;
}
inline bool isOpenDoor(TileType t) {
	return t == TileType::DoorOpenH || t == TileType::DoorOpenV;
}
inline bool isDoorFrontFace(TileType t, bool hitNS) {
	if (t == TileType::DoorClosedH || t == TileType::DoorLocked) return  hitNS;
	if (t == TileType::DoorClosedV)                               return !hitNS;
	return false;
}
inline TileType doorOpenType(TileType t) {
	if (t == TileType::DoorClosedH || t == TileType::DoorLocked) return TileType::DoorOpenH;
	if (t == TileType::DoorClosedV)                               return TileType::DoorOpenV;
	return t;
}
inline TileType doorCloseType(TileType t) {
	if (t == TileType::DoorOpenH) return TileType::DoorClosedH;
	if (t == TileType::DoorOpenV) return TileType::DoorClosedV;
	return t;
}