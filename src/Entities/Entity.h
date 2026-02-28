#pragma once
#include <glm/glm.hpp>
#include <functional>
#include <string>

// ---------------------------------------------------------------------------
// EntityType
// Keep rendering-relevant types (things that need a sprite/decal) separate
// from pure-logic types (monster stages) -- though they can overlap.
// ---------------------------------------------------------------------------
enum class EntityType {
	// World props / interactables
	Note,           // wall-face decal
	Torch,          // floor sprite (future)
	Item,           // collectible floor sprite (future)

	// Monster (single instance; stage drives behaviour + sprite)
	Monster,
};

// Monster assembly stage -- drives AI behaviour and which sprite to use.
enum class MonsterStage {
	None = 0,  // not yet spawned / not yet triggered
	Legs = 1,  // roams aimlessly, no perception
	Arms = 2,  // can open doors, still blind/deaf
	Eyes = 3,  // has line-of-sight, cannot hear
	Ears = 4,  // has line-of-sight + hears sprinting
	// Mouth = 5  // TBD
};

// ---------------------------------------------------------------------------
// Entity
// ---------------------------------------------------------------------------
struct Entity {
	glm::vec2    pos;
	EntityType   type;

	// --- Optional fields (zero-initialised, used by relevant types) ---

	bool         active = true;    // inactive entities are skipped for update + render
	bool         collected = false;   // for collectible items / notes the player has read

	// Wall-face decal: which wall cell this entity renders onto.
	// For floor sprites leave as {-1,-1} and render by world pos instead.
	glm::ivec2   wallCell = { -1, -1 };

	// Monster-specific
	MonsterStage stage = MonsterStage::None;
	glm::vec2    velocity = { 0.0f, 0.0f };

	// Note-specific: text to display when interacted with (future)
	std::string  noteText;

	// Optional interaction callback (called when player presses Use near entity)
	// Stored as a lightweight flag rather than std::function to keep struct simple.
	bool         interactable = false;
};