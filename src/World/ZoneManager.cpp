#include "ZoneManager.h"
#include <iostream>
#include <cmath>

ZoneManager zoneManager;
std::vector<Entity> entities;

void ZoneManager::addTransition(int x, int y, ZoneID target, int entryIdx) {
	int key = y * MAP_W + x;
	transitions[key] = { target, entryIdx };
	setTile(x, y, TileType::ZoneTransition);
}

void ZoneManager::load(ZoneID zone, [[maybe_unused]] int entryIndex) {
	transitions.clear();
	lockedDoors.clear();
	powerRestored = false;
	entities.clear();
	currentZone = zone;
	justTransitioned = true;

	switch (zone) {
	case ZoneID::Zone1: loadZone1(); break;
	}

	std::cout << "[Zone] Loaded zone " << (int)zone
		<< " (" << MAP_W << "x" << MAP_H << ")\n";
}

bool ZoneManager::checkTransitions(Player& player) {
	int tx = (int)player.pos.x;
	int ty = (int)player.pos.y;
	int key = ty * MAP_W + tx;

	if (justTransitioned) {
		if (transitions.find(key) == transitions.end())
			justTransitioned = false;
		return false;
	}

	auto it = transitions.find(key);
	if (it == transitions.end()) return false;

	ZoneTransitionData td = it->second;
	load(td.targetZone, td.entryIndex);

	if (td.entryIndex < (int)entries.size()) {
		const ZoneEntry& e = entries[td.entryIndex];
		player.pos = { e.x, e.y };
		player.angle = e.angle;
	}
	return true;
}
