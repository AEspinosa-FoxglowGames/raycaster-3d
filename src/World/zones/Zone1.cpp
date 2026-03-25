#include "ZoneManager.h"
#include "Entity.h"
#include "Map.h"
#include "Events.h"

// ---------------------------------------------------------------------------
// Zone 1 -- Simple Demo Room
//
// A single square room with:
//   - Stone walls, one locked door on the north wall
//   - Two brick pillars in the center
//   - Candle light (always on)
//   - Two hanging lights (off until fuse puzzle solved)
//   - Desk with a key inside
//   - Fuse pickup on the floor
//   - Fuse box on the east wall (restores power -> lights on)
//   - Wall note
//
// Map (16x16):
//   ################
//   ###D############   D = locked door (north wall, center)
//   #              #
//   #              #
//   #   ##    ##   #   ## = brick pillars
//   #   ##    ##   #
//   #              #
//   #    [c]       #   [c] = candle
//   #       [K]    #   [K] = key pickup
//   #              #
//   #   [D]    [FB]#   [D] = desk   [FB] = fuse box
//   #     [F]      #   [F] = fuse pickup
//   #              #
//   #  [L]    [L]  #   [L] = hanging lights
//   #              #
//   ################
//
// Player spawns at (8, 8) center of room, facing north.
// ---------------------------------------------------------------------------

static Entity makeCandleEntity(glm::vec2 pos);

void loadZone1()
{
	constexpr int W = 16;
	constexpr int H = 16;
	resizeMap(W, H);

	// ---- Outer walls ----
	for (int x = 0; x < W; ++x) {
		setTile(x, 0, TileType::WallStone);
		setTile(x, H - 1, TileType::WallStone);
	}
	for (int y = 0; y < H; ++y) {
		setTile(0, y, TileType::WallStone);
		setTile(W - 1, y, TileType::WallStone);
	}

	// ---- Locked door, north wall center ----
	zoneManager.registerLockedDoor(7, 0);

	// ---- Brick pillars ----
	setTile(3, 4, TileType::WallBrick);
	setTile(3, 5, TileType::WallBrick);
	setTile(4, 4, TileType::WallBrick);
	setTile(4, 5, TileType::WallBrick);

	setTile(10, 4, TileType::WallBrick);
	setTile(10, 5, TileType::WallBrick);
	setTile(11, 4, TileType::WallBrick);
	setTile(11, 5, TileType::WallBrick);

	// ---- Spawn ----
	zoneManager.setEntries({
		{ 8.0f, 8.0f, -1.5707963f },  // center of room, facing north
		});

	// ===================================================================
	// ENTITIES
	// ===================================================================

	// Candle -- always on, warm ambient light
	entities.push_back(makeCandleEntity({ 5.5f, 7.5f }));

	// Hanging lights -- off until fuse box puzzle
	{
		Entity l1 = makeHangingLight({ 3.5f, 13.5f });
		l1.active = false;
		l1.eventTag = (uint8_t)EventTag::Power;
		entities.push_back(l1);

		Entity l2 = makeHangingLight({ 11.5f, 13.5f });
		l2.active = false;
		l2.eventTag = (uint8_t)EventTag::Power;
		entities.push_back(l2);
	}

	// Desk -- contains key (unlocks north door)
	{
		Entity dk = makeDesk({ 3.5f, 10.5f });
		dk.addContent(ItemType::Key);
		entities.push_back(dk);
	}

	// Fuse pickup -- needed for fuse box
	entities.push_back(makeFusePickup({ 6.5f, 11.5f }));

	// Key pickup (spare)
	entities.push_back(makeKey({ 8.5f, 8.5f }));

	// Fuse box -- east wall, restores power
	{
		Entity fb = makeFuseBox({ 14.5f, 10.5f }, 0.8f);
		entities.push_back(fb);
	}

	// Wall note
	{
		Entity n = makeNote(
			{ 1.5f, 8.0f },
			{ 1, 8 },
			"Demo Room\n"
			"Find the key.\n"
			"Restore power.\n"
			"Leave through\n"
			"the north door."
		);
		entities.push_back(n);
	}
}

static Entity makeCandleEntity(glm::vec2 pos)
{
	Entity e;
	e.pos = pos;
	e.type = PropType::Candle;
	e.flags = pflag(PropFlags::Animated);
	e.spriteWorldH = 0.5f;
	e.worldZ = 0.0f;
	e.frameCount = 3;
	e.frameDuration = 0.1f;
	e.loopAnim = true;
	e.lightType = LightType::HangingBulb;
	e.lightRadius = 3.5f;
	e.lightIntensity = 0.6f;
	e.lightColor = { 1.0f, 0.7f, 0.3f };
	e.flickerValue = 1.0f;
	return e;
}