#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include "Inventory.h"

// ---------------------------------------------------------------------------
// PropType
// All entity/prop types. Drives sprite lookup, collision, and behaviour.
// ---------------------------------------------------------------------------
enum class PropType : uint8_t {
	// Wall decals (rendered on wall face, not as billboard)
	Note = 0,
	WallVent = 1,

	// Ceiling / overhead decorative (no collision)
	Camera = 2,
	MirrorShards = 3,

	// Decorative (no collision, no interaction)
	Chains = 10,
	BloodSplatter = 11,
	NoticeBoard = 12,
	Shackles = 13,
	HangingLight = 14,
	FoodTrayFull = 15,
	FoodTrayEmpty = 16,
	Cup = 17,
	Drain = 18,
	BulletinBoard = 19,

	// Collideable, not interactable
	Bed = 20,
	Toilet = 21,
	Shower = 22,
	Stool = 23,
	Sink = 24,
	Chair1 = 25,
	Chair2 = 26,
	Shelves = 27,
	InterrogationChair = 28,
	Urinal = 29,

	// Collideable + interactable
	Locker = 30,
	Desk = 31,
	Lever = 32,
	GuardStation = 33,
	AutopsyTable = 34,
	InterrogationTable = 35,
	Gurney = 36,
	IVDrip = 37,
	Locker2 = 38,
	Desk2 = 39,

	// Animated decorative
	Candle = 40,
	Torch = 41,

	// Collideable, not interactable (cont.)
	Crates = 42,
	MonitorStack = 43,
	Mop = 44,
	TrashCan = 45,
	FireExtinguisher = 46,

	// Collideable + interactable (cont.)
	Mirror = 47,
	Radio = 48,
	KeyRack = 49,
	ControlPanel = 50,
	Safe = 51,
	FuseBox = 52,
	FirstAidKit = 53,
	FusePickup = 54,

	// Collectibles
	Item = 60,
	Logbook = 61,
	Baton = 62,
	Flashlight = 63,
	Key = 64,

	// Internal / non-rendered
	PlayerLight = 255,
};

enum class LightType : uint8_t {
	None = 0,
	HangingBulb,    // aggressive stutter flicker
	WallSconce,     // slow warm pulse
	Ambient,        // no flicker, e.g. exit sign glow
	Flashlight,     // subtle wobble; random cut-out handled in main
};

// ---------------------------------------------------------------------------
// PropFlags  (stored as uint8_t bitmask in Entity)
// ---------------------------------------------------------------------------
enum class PropFlags : uint8_t {
	None        = 0,
	Collideable  = 1 << 0,
	Interactable = 1 << 1,
	Animated     = 1 << 2,
	WallDecal    = 1 << 3,
	Stackable    = 1 << 4,
};
inline uint8_t pflag(PropFlags f) { return static_cast<uint8_t>(f); }
inline uint8_t operator|(PropFlags a, PropFlags b) { return pflag(a) | pflag(b); }
inline uint8_t operator|(uint8_t  a, PropFlags b) { return a | pflag(b); }
inline bool    hasFlag(uint8_t flags, PropFlags f) { return (flags & pflag(f)) != 0; }

// ---------------------------------------------------------------------------
// Entity
// ---------------------------------------------------------------------------
struct Entity {
	glm::vec2    pos = { 0.0f, 0.0f };
	PropType     type = PropType::Note;
	uint8_t      flags = 0;
	bool         active = true;

	// ---- Vertical placement (world units; WALL_HEIGHT = 4.0) ----
	float        worldZ = 0.0f;
	float        spriteWorldH = 2.0f;
	float        stackTop = 0.0f;

	// ---- Collision ----
	float        collideRadius = 0.0f;

	// ---- Animation ----
	int          frameCount = 1;
	int          currentFrame = 0;
	float        frameDuration = 0.125f;
	float        frameTimer = 0.0f;
	bool         loopAnim = true;
	bool         mirrorX = false;

	// ---- Interaction state ----
	bool         isOpen = false;
	bool         isLocked = false;
	bool         isUsed = false;
	uint8_t      keyID = 0;
	std::string  interactPrompt;

	// ---- Event system ----
	uint8_t      onUseEvent = 0;
	uint8_t      eventTag = 0;

	// ---- Loot contents ----
	std::array<ItemType, 4> contents{};
	int contentCount = 0;
	void addContent(ItemType item) {
		if (contentCount < 4) contents[contentCount++] = item;
	}

	// ---- Wall decal fields ----
	glm::ivec2   wallCell = { -1, -1 };
	std::string  noteText;

	// ---- Lighting ----
	LightType  lightType = LightType::None;
	float      lightRadius = 0.0f;
	float      lightIntensity = 0.0f;
	glm::vec3  lightColor = { 1.0f, 0.9f, 0.75f };
	float      flickerValue = 1.0f;
	float      flickerTimer = 0.0f;
};

// ---------------------------------------------------------------------------
// Factory helpers
// ---------------------------------------------------------------------------

inline Entity makeNote(glm::vec2 pos, glm::ivec2 cell, const std::string& text)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Note;
	e.flags = PropFlags::WallDecal | PropFlags::Interactable;
	e.wallCell = cell;
	e.noteText = text;
	e.interactPrompt = "Read note";
	return e;
}

inline Entity makeBed(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Bed;
	e.flags = pflag(PropFlags::Collideable);
	e.spriteWorldH = 1.6f;
	e.collideRadius = 0.55f;
	return e;
}

inline Entity makeLocker(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Locker;
	e.flags = PropFlags::Collideable | PropFlags::Interactable | PropFlags::Stackable;
	e.spriteWorldH = 3.2f;
	e.collideRadius = 0.35f;
	e.interactPrompt = "Open locker";
	return e;
}

inline Entity makeLockedLocker(glm::vec2 pos, uint8_t keyID = 0)
{
	Entity e = makeLocker(pos);
	e.isLocked = true;
	e.keyID = keyID;
	e.interactPrompt = "Locked";
	return e;
}

inline Entity makeDesk(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Desk;
	e.flags = PropFlags::Collideable | PropFlags::Interactable | PropFlags::Stackable;
	e.spriteWorldH = 1.75f;
	e.collideRadius = 0.5f;
	e.stackTop = 1.0f;
	e.interactPrompt = "Search desk";
	return e;
}

inline Entity makeLockedDesk(glm::vec2 pos, uint8_t keyID = 0)
{
	Entity e = makeDesk(pos);
	e.isLocked = true;
	e.keyID = keyID;
	e.interactPrompt = "Locked";
	return e;
}

inline Entity makeStool(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Stool;
	e.flags = pflag(PropFlags::Collideable);
	e.spriteWorldH = 0.7f;
	e.collideRadius = 0.25f;
	return e;
}

inline Entity makeToilet(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Toilet;
	e.flags = pflag(PropFlags::Collideable);
	e.spriteWorldH = 1.5f;
	e.collideRadius = 0.3f;
	return e;
}

inline Entity makeShower(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Shower;
	e.flags = pflag(PropFlags::Collideable);
	e.spriteWorldH = 3.5f;
	e.collideRadius = 0.4f;
	return e;
}

inline Entity makeMirror(glm::vec2 pos, float worldZ = 0.8f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Mirror;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 2.2f;
	e.interactPrompt = "Examine mirror";
	return e;
}

inline Entity makeRadio(glm::vec2 pos, float worldZ = 0.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Radio;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 1.2f;
	e.interactPrompt = "Use radio";
	return e;
}

inline Entity makeKeyRack(glm::vec2 pos, float worldZ = 1.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::KeyRack;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 2.0f;
	e.interactPrompt = "Search key rack";
	return e;
}

inline Entity makeControlPanel(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::ControlPanel;
	e.flags = PropFlags::Collideable | PropFlags::Interactable | PropFlags::Stackable;
	e.spriteWorldH = 2.6f;
	e.collideRadius = 0.7f;
	e.stackTop = 1.4f;
	e.interactPrompt = "Use panel";
	return e;
}

inline Entity makeSafe(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Safe;
	e.flags = PropFlags::Collideable | PropFlags::Interactable;
	e.spriteWorldH = 2.0f;
	e.collideRadius = 0.45f;
	e.interactPrompt = "Open safe";
	return e;
}

inline Entity makeLockedSafe(glm::vec2 pos, uint8_t keyID = 0)
{
	Entity e = makeSafe(pos);
	e.isLocked = true;
	e.keyID = keyID;
	e.interactPrompt = "Locked";
	return e;
}

inline Entity makeLogbook(glm::vec2 pos, float worldZ = 0.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Logbook;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 0.5f;
	e.interactPrompt = "Read logbook";
	return e;
}

inline Entity makeBaton(glm::vec2 pos, float worldZ = 0.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Baton;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 0.4f;
	e.interactPrompt = "Pick up baton";
	return e;
}

inline Entity makeFlashlight(glm::vec2 pos, float worldZ = 0.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Flashlight;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 0.4f;
	e.interactPrompt = "Pick up flashlight";
	return e;
}

inline Entity makeTrashCan(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::TrashCan;
	e.flags = pflag(PropFlags::Collideable);
	e.spriteWorldH = 1.6f;
	e.collideRadius = 0.3f;
	return e;
}

inline Entity makeFireExtinguisher(glm::vec2 pos, float worldZ = 0.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::FireExtinguisher;
	e.flags = pflag(PropFlags::Collideable);
	e.worldZ = worldZ;
	e.spriteWorldH = 2.0f;
	e.collideRadius = 0.2f;
	return e;
}

inline Entity makeFuseBox(glm::vec2 pos, float worldZ = 0.6f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::FuseBox;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 2.2f;
	e.interactPrompt = "Examine fuse box";
	return e;
}

inline Entity makeFirstAidKit(glm::vec2 pos, float worldZ = 0.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::FirstAidKit;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 0.9f;
	e.interactPrompt = "Use first aid kit";
	return e;
}

inline Entity makeFusePickup(glm::vec2 pos, float worldZ = 0.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::FusePickup;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 0.6f;
	e.interactPrompt = "Take fuse";
	return e;
}

inline Entity makeKey(glm::vec2 pos, float worldZ = 0.0f, uint8_t keyID = 0)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Key;
	e.flags = pflag(PropFlags::Interactable);
	e.worldZ = worldZ;
	e.spriteWorldH = 0.35f;
	e.keyID = keyID;
	e.interactPrompt = "Pick up key";
	return e;
}

inline Entity makeHangingLight(glm::vec2 pos, float worldZ = 3.0f, float intensity = 1.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::HangingLight;
	e.flags = 0;
	e.worldZ = worldZ;
	e.spriteWorldH = 1.0f;
	e.lightType = LightType::HangingBulb;
	e.lightRadius = 6.0f;
	e.lightIntensity = intensity;
	e.lightColor = { 1.0f, 0.85f, 0.6f };
	e.flickerValue = 1.0f;
	e.flickerTimer = 0.0f;
	return e;
}

inline Entity makeHangingLightOFF(glm::vec2 pos, float worldZ = 3.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::HangingLight;
	e.flags = 0;
	e.worldZ = worldZ;
	e.spriteWorldH = 1.0f;
	return e;
}

// ---------------------------------------------------------------------------
// makePlayerFlashlight
// Invisible light entity that follows the player. Position updated each frame.
// ---------------------------------------------------------------------------
inline Entity makePlayerFlashlight()
{
	Entity e;
	e.pos = { 0.0f, 0.0f };
	e.type = PropType::PlayerLight;
	e.flags = 0;
	e.active = false;
	e.spriteWorldH = 0.0f;
	e.lightType = LightType::Flashlight;
	e.lightRadius = 9.0f;
	e.lightIntensity = 0.55f;
	e.lightColor = { 0.88f, 0.92f, 1.0f };
	e.flickerValue = 1.0f;
	e.flickerTimer = 0.0f;
	return e;
}

inline Entity makeWallVent(glm::vec2 pos, float worldZ = 1.5f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::WallVent;
	e.flags = pflag(PropFlags::WallDecal);
	e.worldZ = worldZ;
	e.spriteWorldH = 0.6f;
	return e;
}

inline Entity makeTrayEmpty(glm::vec2 pos, float worldZ = 0.0f)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::FoodTrayEmpty;
	e.flags = 0;
	e.worldZ = worldZ;
	e.spriteWorldH = 0.25f;
	return e;
}

// Aliases
inline Entity makeFusebox(glm::vec2 pos, float worldZ = 0.6f) { return makeFuseBox(pos, worldZ); }
inline Entity makeFirstAid(glm::vec2 pos, float worldZ = 0.0f) { return makeFirstAidKit(pos, worldZ); }
inline Entity makeTrashcan(glm::vec2 pos) { return makeTrashCan(pos); }

// ---------------------------------------------------------------------------
// updateEntities -- call once per game-loop frame.
// ---------------------------------------------------------------------------
inline void updateEntities(std::vector<Entity>& entities, float dt)
{
	for (auto& e : entities)
	{
		if (!e.active) continue;
		if (!hasFlag(e.flags, PropFlags::Animated)) continue;
		if (e.frameCount <= 1) continue;

		e.frameTimer += dt;
		if (e.frameTimer >= e.frameDuration)
		{
			e.frameTimer -= e.frameDuration;
			if (e.loopAnim)
				e.currentFrame = (e.currentFrame + 1) % e.frameCount;
			else
				e.currentFrame = std::min(e.currentFrame + 1, e.frameCount - 1);
		}
	}
}
