#pragma once
#include <glm/glm.hpp>
#include <cstdint>

// ---------------------------------------------------------------------------
// ZoneID
// Add new zones here and implement a corresponding loadZoneN() function
// in a ZoneN.cpp file, then register it in ZoneManager::load().
// ---------------------------------------------------------------------------
enum class ZoneID : uint8_t {
	Zone1 = 0,
	Zone2 = 1,
	Zone3 = 2,
	Zone4 = 3,
	Zone5 = 4,
	Zone6 = 5,
};

// Each zone can have multiple entry points (e.g. coming from north vs south)
struct ZoneEntry {
	float x, y;     // player spawn position
	float angle;    // spawn facing angle (radians)
};

struct ZoneTransitionData {
	ZoneID targetZone;
	int    entryIndex;  // which ZoneEntry to use in target zone
};
