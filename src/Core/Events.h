#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "Map.h"
#include "ZoneManager.h"
#include "hud.h"

// Forward declarations
struct Entity;
class HUD;

// ---------------------------------------------------------------------------
// EventID
// Named game events. Each interactablee entity can carry one via onUseEvent.
// Add new entries here as needed -- fireEvent() handles them all.
// ---------------------------------------------------------------------------
enum class EventID : uint8_t {
	None = 0,

	// Door/access events -- unlock specific doors by tile coordinate
	UnlockDoorA,        // first major door unlock
	UnlockExit,         // final unlock: converts a tile to ZoneExit
	UnlockDoorB,        // secondary door unlock

	// Entity events -- toggle/activate entities by tag
	PowerOn,            // activates all entities with EventTag::Power
	PowerOff,
	OpenGates,          // opens all entities tagged EventTag::Gate

	// Puzzle events
	FuseRestored,       // fuse box repaired, chains into PowerOn
	AlarmTriggered,     // placeholder for downstream logic
};

// ---------------------------------------------------------------------------
// EventTag
// Entities can be tagged so events can target groups of them.
// ---------------------------------------------------------------------------
enum class EventTag : uint8_t {
	None    = 0,
	Power   = 1,   // lighting / powered props
	Gate    = 2,   // doors that open as a group
	LabDoor = 3,
};

// ---------------------------------------------------------------------------
// fireEvent
// Central dispatcher. Called from tryInteractEntity when an entity's
// onUseEvent fires. Modifies map tiles and entities in-place.
//
// Replace the placeholder tile coordinates below with your actual map coords.
// Add a new case here whenever you add a new EventID.
// ---------------------------------------------------------------------------
inline void fireEvent(
	EventID              id,
	std::vector<Entity>& entities,
	HUD&                 hud)
{
	switch (id)
	{
	// ---- Door A unlock --------------------------------------------------------
	case EventID::UnlockDoorA:
	{
		// Replace (0, 0) with your target door tile coordinates
		zoneManager.unlockDoor(0, 0);
		hud.pushNotification("Door unlocked");
		break;
	}

	// ---- Exit unlock ----------------------------------------------------------
	case EventID::UnlockExit:
	{
		// Replace (0, 0) with your exit tile coordinates
		setTile(15, 13, TileType::ZoneExit);
		hud.pushNotification("Exit unsealed");
		break;
	}

	// ---- Door B unlock --------------------------------------------------------
	case EventID::UnlockDoorB:
	{
		// Replace (0, 0) with your target door tile coordinates
		zoneManager.unlockDoor(0, 0);
		hud.pushNotification("Door unlocked");
		break;
	}

	// ---- Power on -------------------------------------------------------------
	case EventID::PowerOn:
	{
		zoneManager.powerRestored = true;
		for (auto& e : entities)
			if (e.eventTag == (uint8_t)EventTag::Power)
				e.active = true;
		hud.pushNotification("Power restored");
		break;
	}

	// ---- Power off ------------------------------------------------------------
	case EventID::PowerOff:
	{
		for (auto& e : entities)
			if (e.eventTag == (uint8_t)EventTag::Power)
				e.active = false;
		hud.pushNotification("Power cut");
		break;
	}

	// ---- Open gates -----------------------------------------------------------
	case EventID::OpenGates:
	{
		for (auto& e : entities)
			if (e.eventTag == (uint8_t)EventTag::Gate && !e.isOpen) {
				e.isOpen = true;
				e.interactPrompt = "Close door";
			}
		hud.pushNotification("Gates released");
		break;
	}

	// ---- Fuse restored --------------------------------------------------------
	case EventID::FuseRestored:
		hud.pushNotification("Power circuit restored");
		fireEvent(EventID::PowerOn, entities, hud);
		break;

	// ---- Alarm ----------------------------------------------------------------
	case EventID::AlarmTriggered:
		hud.pushNotification("ALARM TRIGGERED");
		break;

	default:
		break;
	}
}
