#pragma once
#include "Zone.h"
#include "Map.h"
#include "Entity.h"
#include "Player.h"
#include <vector>
#include <unordered_map>
#include <functional>

// Per-zone load functions -- implement one .cpp per zone.
// Example: Zone1.cpp defines loadZone1(), populates MAP and entities.
void loadZone1();

class ZoneManager {
public:
	ZoneID currentZone = ZoneID::Zone1;

	std::unordered_map<int, ZoneTransitionData> transitions;
	std::vector<ZoneEntry> entries;

	// Locked door registry: maps tile coord key → keyID required to open.
	// keyID 0 = any Key item works. keyID > 0 = must match specific key ID.
	std::unordered_map<int, uint8_t> lockedDoors;

	bool powerRestored = false;   // set true by PowerOn event; gates terminals

	void load(ZoneID zone, int entryIndex = 0);
	bool checkTransitions(Player& player);
	void addTransition(int x, int y, ZoneID target, int entryIdx);
	void setEntries(std::vector<ZoneEntry> e) { entries = std::move(e); }

	void registerLockedDoor(int x, int y, uint8_t keyID = 0) {
		lockedDoors[y * MAP_W + x] = keyID;
		setTile(x, y, TileType::DoorLocked);
	}

	void unlockDoor(int x, int y) {
		lockedDoors.erase(y * MAP_W + x);
		TileType t = getTile(x, y);
		if (t == TileType::DoorLocked)
			setTile(x, y, TileType::DoorClosedH);
	}

	int getLockedDoorKeyID(int x, int y) const {
		auto it = lockedDoors.find(y * MAP_W + x);
		if (it == lockedDoors.end()) return 0;
		return it->second;
	}

private:
	bool justTransitioned = false;
};

extern ZoneManager zoneManager;
extern std::vector<Entity> entities;
