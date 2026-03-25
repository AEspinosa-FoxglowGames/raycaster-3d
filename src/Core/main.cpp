#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <cstdlib>
#include "Map.h"
#include "Player.h"
#include "Shader.h"
#include "Raycaster.h"
#include "Texture.h"
#include "Renderer.h"
#include "HUD.h"
#include "ZoneManager.h"
#include "Inventory.h"
#include "Lighting.h"
#include "Events.h"
#include "Journal.h"

constexpr float INTERACT_REACH = 2.0f;
constexpr float PLAYER_RADIUS = 0.25f;

// Probability per second that a switched-on flashlight randomly cuts out.
// At 2% the average runtime is ~50 seconds.  Tune as desired.
constexpr float FLASHLIGHT_FAIL_CHANCE = 0.02f;

inline ItemType propToItem(PropType t) {
	switch (t) {
	case PropType::Key:         return ItemType::Key;
	case PropType::Flashlight:  return ItemType::Flashlight;
	case PropType::Baton:       return ItemType::Baton;
	case PropType::FirstAidKit: return ItemType::FirstAidKit;
	case PropType::Logbook:     return ItemType::Logbook;
	case PropType::FusePickup:  return ItemType::Fuse;
	default:                    return ItemType::None;
	}
}

// ---------------------------------------------------------------------------
// tryInteractDoor
// Checks the zoneManager.lockedDoors registry for keyID requirements.
// keyID 0 = any Key item.  keyID >0 = must match a Key with that ID.
// ---------------------------------------------------------------------------
bool tryInteractDoor(Player& player, HUD& hud)
{
	RayHit hit = castRay(player.pos, player.angle);
	if (hit.dist > INTERACT_REACH) return false;

	int wx = hit.wallX, wy = hit.wallY;
	TileType t = getTile(wx, wy);

	if (isDoor(t)) {
		if (t == TileType::DoorLocked) {
			if (player.inventory.has(ItemType::Key)) {
				player.inventory.remove(ItemType::Key);
				zoneManager.lockedDoors.erase(wy * MAP_W + wx);
				setTile(wx, wy, TileType::DoorClosedH);
				hud.pushNotification("Door unlocked");
				return true;
			}
			hud.pushNotification("Locked -- need a key");
			return true;
		}
		setTile(wx, wy, doorOpenType(t));
		return true;
	}
	if (isOpenDoor(t)) {
		int px = (int)player.pos.x;
		int py = (int)player.pos.y;
		if (px == wx && py == wy) return true;
		setTile(wx, wy, doorCloseType(t));
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// tryInteractNote
// ---------------------------------------------------------------------------
bool tryInteractNote(Player& player, std::vector<Entity>& entities, HUD& hud)
{
	RayHit hit = castRay(player.pos, player.angle);
	if (hit.dist > INTERACT_REACH) return false;

	for (auto& e : entities) {
		if (!e.active) continue;
		if (e.type != PropType::Note) continue;
		if (e.wallCell.x != hit.wallX || e.wallCell.y != hit.wallY) continue;

		std::string title = e.noteText;
		auto nl = title.find('\n');
		if (nl != std::string::npos) title = title.substr(0, nl);
		if (title.size() > 20) title = title.substr(0, 20);

		player.journal.add(title, e.noteText);
		hud.pushNotification("Note added to journal  [J]");
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// tryUnlockEntity
// ---------------------------------------------------------------------------
static bool tryUnlockEntity(Player& player, Entity& e)
{
	if (!e.isLocked) return true;

	if (!player.inventory.has(ItemType::Key)) {
		std::cout << "[Lock] Locked. You need a key.\n";
		return false;
	}

	player.inventory.remove(ItemType::Key);
	e.isLocked = false;
	std::cout << "[Lock] Unlocked with key.\n";
	return true;
}

// ---------------------------------------------------------------------------
// giveItem
// ---------------------------------------------------------------------------
static bool giveItem(Player& player, HUD& hud, ItemType item, const char* source)
{
	if (item == ItemType::None) return false;

	if (item == ItemType::FirstAidKit) {
		player.health = std::min(player.health + 50, player.maxHealth);
		hud.pushNotification("Used first aid kit");
		return true;
	}

	if (!player.inventory.add(item)) {
		hud.pushNotification(std::string(itemName(item)) + " -- inventory full");
		return false;
	}

	hud.pushNotification(std::string("Found: ") + itemName(item));

	switch (item) {
	case ItemType::Flashlight:
		player.flashlightOn = true;
		hud.pushNotification("Press F to toggle flashlight");
		player.journal.add("Flashlight",
			"Found a flashlight.\nPress F to toggle it.\nBattery seems weak.");
		break;
	case ItemType::Key:
		player.journal.add("A Key",
			"Found a key.\nThere must be a locked\ndoor or container nearby.");
		break;
	case ItemType::Baton:
		player.journal.add("Baton",
			"A heavy baton.\nSomeone left it behind.");
		break;
	case ItemType::Logbook:
		player.journal.add("Logbook",
			"A logbook.\nMost entries are routine.\nThe last page is torn out.");
		break;
	default: break;
	}

	return true;
}

// ---------------------------------------------------------------------------
// lootContainer
// ---------------------------------------------------------------------------
static void lootContainer(Player& player, HUD& hud, Entity& e, const char* label)
{
	if (e.contentCount == 0) { hud.pushNotification(std::string(label) + " is empty"); return; }
	int remaining = 0;
	for (int i = 0; i < e.contentCount; ++i) {
		if (!giveItem(player, hud, e.contents[i], label))
			e.contents[remaining++] = e.contents[i];
	}
	e.contentCount = remaining;
}

// ---------------------------------------------------------------------------
// tryInteractEntity
// ---------------------------------------------------------------------------
Entity* tryInteractEntity(Player& player, HUD& hud, std::vector<Entity>& entities)
{
	Entity* best = nullptr;
	float   bestDist = INTERACT_REACH;
	glm::vec2 forward = { cosf(player.angle), sinf(player.angle) };

	for (auto& e : entities) {
		if (!e.active) continue;
		if (!hasFlag(e.flags, PropFlags::Interactable)) continue;
		if (hasFlag(e.flags, PropFlags::WallDecal)) continue;

		glm::vec2 toEntity = e.pos - player.pos;
		float dist = glm::length(toEntity);
		if (dist >= bestDist) continue;
		float dot = glm::dot(forward, toEntity / dist);
		if (dot < 0.7f) continue;

		bestDist = dist;
		best = &e;
	}

	if (!best) return nullptr;

	ItemType item = propToItem(best->type);
	if (item != ItemType::None) {
		if (giveItem(player, hud, item, "Item"))
			best->active = false;
		return best;
	}

	switch (best->type)
	{
	case PropType::Locker:
	case PropType::Locker2:
	{
		if (!tryUnlockEntity(player, *best)) return best;
		best->isOpen = !best->isOpen;
		best->interactPrompt = best->isOpen ? "Close locker" : "Open locker";
		if (best->isOpen) lootContainer(player, hud, *best, "Locker");
		break;
	}
	case PropType::Safe:
	{
		if (!tryUnlockEntity(player, *best)) return best;
		best->isOpen = !best->isOpen;
		best->interactPrompt = best->isOpen ? "Close safe" : "Open safe";
		if (best->isOpen) lootContainer(player, hud, *best, "Safe");
		break;
	}
	case PropType::Desk:
	case PropType::Desk2:
	case PropType::GuardStation:
	{
		if (!tryUnlockEntity(player, *best)) return best;
		best->isOpen = !best->isOpen;
		best->interactPrompt = best->isOpen ? "Close" : "Search desk";
		if (best->isOpen) lootContainer(player, hud, *best, "Desk");
		break;
	}
	case PropType::KeyRack:
	{
		lootContainer(player, hud, *best, "Key Rack");
		if (best->contentCount == 0)
			best->flags &= ~pflag(PropFlags::Interactable);
		break;
	}
	case PropType::Lever:
		best->isOpen = !best->isOpen;
		best->interactPrompt = best->isOpen ? "Reset lever" : "Pull lever";
		break;
	case PropType::Logbook:
		hud.pushNotification("You read the logbook");
		break;
	case PropType::Mirror:
		hud.pushNotification("Something stares back");
		break;
	case PropType::Radio:
		best->isOpen = !best->isOpen;
		best->interactPrompt = best->isOpen ? "Turn off radio" : "Turn on radio";
		break;
	case PropType::BulletinBoard:
		hud.pushNotification("Pinned notes, mostly illegible");
		break;
	case PropType::FuseBox:
		if (best->isUsed) {
			hud.pushNotification("Power is on");
		}
		else if (player.inventory.has(ItemType::Fuse)) {
			player.inventory.remove(ItemType::Fuse);
			best->isUsed = true;
			best->interactPrompt = "Fuse box (active)";
			best->flags &= ~pflag(PropFlags::Interactable);
			hud.pushNotification("Fuse replaced -- power restored");
			fireEvent(EventID::PowerOn, entities, hud);
		}
		else {
			hud.pushNotification("Burned-out fuse. Need a replacement");
			player.journal.add("Fuse Box",
				"The fuse box is dead.\nA fuse is burned out.\nMaybe there's a spare nearby.");
		}
		break;
	case PropType::AutopsyTable:
		hud.pushNotification("Cold steel. Whatever was here is gone");
		break;
	case PropType::InterrogationTable:
		hud.pushNotification("Scratches and dried stains cover the surface");
		break;
	case PropType::Gurney:
		hud.pushNotification("A restraint strap hangs loose");
		break;
	case PropType::ControlPanel:
		if (!zoneManager.powerRestored) {
			hud.pushNotification("Terminal offline -- no power");
		}
		else if (best->onUseEvent == (uint8_t)EventID::None) {
			hud.pushNotification("The controls are locked out");
		}
		break;
	default:
		break;
	}

	bool canFire = (best->onUseEvent != (uint8_t)EventID::None);
	if (canFire && best->type == PropType::ControlPanel && !zoneManager.powerRestored)
		canFire = false;

	if (canFire) {
		fireEvent((EventID)best->onUseEvent, entities, hud);
		best->onUseEvent = (uint8_t)EventID::None;
		best->isUsed = true;
		best->interactPrompt = "";
		best->flags &= ~pflag(PropFlags::Interactable);
	}

	return best;
}

// ---------------------------------------------------------------------------
// applyPropCollision
// ---------------------------------------------------------------------------
void applyPropCollision(glm::vec2& newPos, const std::vector<Entity>& entities)
{
	for (const auto& e : entities) {
		if (!e.active) continue;
		if (!hasFlag(e.flags, PropFlags::Collideable)) continue;

		glm::vec2 diff = newPos - e.pos;
		float dist = glm::length(diff);
		float minDist = e.collideRadius + PLAYER_RADIUS;

		if (dist < minDist && dist > 0.001f)
			newPos = e.pos + glm::normalize(diff) * minDist;
	}
}

// ---------------------------------------------------------------------------
// buildPromptText
// ---------------------------------------------------------------------------
static std::string buildPromptText(const Player& player,
	const std::vector<Entity>& entities)
{
	glm::vec2 forward = { cosf(player.angle), sinf(player.angle) };
	std::string text;
	float       best = INTERACT_REACH;

	for (const auto& e : entities) {
		if (!e.active || !hasFlag(e.flags, PropFlags::Interactable)) continue;
		if (hasFlag(e.flags, PropFlags::WallDecal)) continue;
		glm::vec2 toE = e.pos - player.pos;
		float d = glm::length(toE);
		if (d >= best) continue;
		if (glm::dot(forward, toE / d) < 0.7f) continue;
		best = d;

		if (e.isLocked) {
			text = player.inventory.has(ItemType::Key) ? "Unlock [Key]" : "Locked";
		}
		else {
			text = e.interactPrompt;
		}
	}

	RayHit dh = castRay(player.pos, player.angle);
	if (dh.dist <= INTERACT_REACH) {
		if (isDoor(dh.tile))
			text = (dh.tile == TileType::DoorLocked)
			? (player.inventory.has(ItemType::Key) ? "Unlock door [Key]" : "Locked door")
			: "Open door";
		else if (isOpenDoor(dh.tile))
			text = "Close door";
	}

	return text;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	constexpr int SCREEN_W = 800;
	constexpr int SCREEN_H = 600;
	constexpr int WINDOW_W = 1600;
	constexpr int WINDOW_H = 1200;

	GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H,
		"Raycaster Engine Demo", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, WINDOW_W, WINDOW_H);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	static int scrollDelta = 0;
	glfwSetWindowUserPointer(window, &scrollDelta);
	glfwSetScrollCallback(window, [](GLFWwindow* w, double /*xoff*/, double yoff) {
		int* delta = static_cast<int*>(glfwGetWindowUserPointer(w));
		*delta += (yoff < 0) ? 1 : -1;
		});

	Renderer renderer(SCREEN_W, SCREEN_H,
		"assets/shaders/screen.vert",
		"assets/shaders/screen.frag");
	HUD hud(renderer, SCREEN_W, SCREEN_H);

	// ---- Textures ----
	TextureRegistry textures;

	// ---- Wall textures ----
	textures.setWall(TileType::WallStone,    loadCPUTexture("assets/textures/walls/wall.png"));
	textures.setWall(TileType::WallBrick,    loadCPUTexture("assets/textures/walls/BlackBrick.jpg"));
	textures.setWall(TileType::WallConcrete, loadCPUTexture("assets/textures/walls/GrayConcrete.jpg"));
	textures.setWall(TileType::WallSealed,   loadCPUTexture("assets/textures/walls/StoneTile.png"));
	textures.setWall(TileType::WallGlass,    loadCPUTexture("assets/textures/walls/StoneTile.png")); // swap for Glass.png if available
	textures.setWall(TileType::DoorClosedH,  loadCPUTexture("assets/textures/doors/creepydoor.jpg"));
	textures.setWall(TileType::DoorClosedV,  loadCPUTexture("assets/textures/doors/creepydoor.jpg"));
	textures.setWall(TileType::DoorLocked,   loadCPUTexture("assets/textures/doors/metaldoor.jpg"));
	textures.setWall(TileType::DoorFrame,    loadCPUTexture("assets/textures/walls/GrayConcrete.jpg"));
	textures.setWall(TileType::DoorOpenH,    loadCPUTexture("assets/textures/doors/creepydoor.jpg"));
	textures.setWall(TileType::DoorOpenV,    loadCPUTexture("assets/textures/doors/creepydoor.jpg"));
	textures.floorTex   = loadCPUTexture("assets/textures/walls/StoneTile.png");
	textures.ceilingTex = loadCPUTexture("assets/textures/walls/GrayConcrete.jpg");

	// ---- Sprites (minimal demo set) ----
	textures.setSprite(PropType::HangingLight, loadCPUTexture("assets/sprites/Light.png"));
	textures.setSprite(PropType::Desk,         loadCPUTexture("assets/sprites/Desk.png"));
	textures.setSprite(PropType::Crates,       loadCPUTexture("assets/sprites/crates.png"));
	textures.setSprite(PropType::FuseBox,      loadCPUTexture("assets/sprites/fusebox.png"));
	textures.setSprite(PropType::FusePickup,   loadCPUTexture("assets/sprites/key.png"));
	textures.setSprite(PropType::Key,          loadCPUTexture("assets/sprites/key.png"));

	// Animated: candle (3 frames at 10 fps)
	textures.setSpriteSheet(PropType::Candle,
		makeSpriteSheet({ "assets/sprites/candle/candle0.png",
						  "assets/sprites/candle/candle1.png",
						  "assets/sprites/candle/candle2.png" }, 10.0f));

	// ---- Load starting zone ----
	zoneManager.load(ZoneID::Zone1, 0);

	// ---- Player flashlight light entity ----
	entities.push_back(makePlayerFlashlight());
	int flashlightEntityIdx = (int)entities.size() - 1;

	Player player;
	float  lastTime = (float)glfwGetTime();
	double lastX = WINDOW_W / 2.0;
	double lastY = WINDOW_H / 2.0;
	constexpr float MOUSE_SENSITIVITY = 0.001f;

	bool ePrevPressed    = false;
	bool tabPrevPressed  = false;
	bool fPrevPressed    = false;
	bool iPrevPressed    = false;
	bool jPrevPressed    = false;
	bool upPrevPressed   = false;
	bool downPrevPressed = false;

	while (!glfwWindowShouldClose(window))
	{
		float now = (float)glfwGetTime();
		float dt = now - lastTime;
		lastTime = now;
		dt = std::min(dt, 0.05f);

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// Debug: hurt player
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
			player.health = std::max(0, player.health - 1);

		// ---- Mouse look ----
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		player.angle += (float)(xpos - lastX) * MOUSE_SENSITIVITY;
		player.pitch -= (float)(ypos - lastY) * MOUSE_SENSITIVITY * 200.0f;
		player.pitch = glm::clamp(player.pitch, -player.pitchMax, player.pitchMax);
		lastX = xpos; lastY = ypos;

		// ---- Movement ----
		glm::vec2 forward = { cosf(player.angle), sinf(player.angle) };
		glm::vec2 right = { -forward.y, forward.x };
		glm::vec2 newPos = player.pos;

		bool sprinting = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
			&& player.stamina > 0.0f && !player.exhausted;

		float moveSpeed = sprinting ? player.sprintSpeed : player.speed;
		if (glfwGetWindowAttrib(window, GLFW_FOCUSED))
		{
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) newPos += forward * moveSpeed * dt;
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) newPos -= forward * moveSpeed * dt;
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) newPos -= right  * moveSpeed * dt;
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) newPos += right  * moveSpeed * dt;
		}
		player.running = sprinting;

		if (sprinting) player.stamina = glm::clamp(player.stamina - 15.0f * dt, 0.0f, (float)player.maxStamina);
		else           player.stamina = glm::clamp(player.stamina +  5.0f * dt, 0.0f, (float)player.maxStamina);

		if (player.stamina <= 0.0f)                      player.exhausted = true;
		if (player.stamina >= player.maxStamina * 0.25f) player.exhausted = false;

		// ---- Collision ----
		applyPropCollision(newPos, entities);

		glm::vec2 testX = { newPos.x, player.pos.y };
		glm::vec2 testY = { player.pos.x, newPos.y };
		bool xClear = !isSolid(getTile((int)(testX.x + PLAYER_RADIUS), (int)testX.y))
			&& !isSolid(getTile((int)(testX.x - PLAYER_RADIUS), (int)testX.y));
		if (xClear) player.pos.x = testX.x;

		bool yClear = !isSolid(getTile((int)testY.x, (int)(testY.y + PLAYER_RADIUS)))
			&& !isSolid(getTile((int)testY.x, (int)(testY.y - PLAYER_RADIUS)));
		if (yClear) player.pos.y = testY.y;

		// ---- Zone transition ----
		if (zoneManager.checkTransitions(player)) {
			entities.push_back(makePlayerFlashlight());
			flashlightEntityIdx = (int)entities.size() - 1;
			if (player.inventory.has(ItemType::Flashlight))
				entities[flashlightEntityIdx].active = player.flashlightOn;
			continue;
		}

		// ---- Interact (E) ----
		bool eNow = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
		if (eNow && !ePrevPressed) {
			if (!tryInteractDoor(player, hud))
				if (!tryInteractNote(player, entities, hud))
					tryInteractEntity(player, hud, entities);
		}
		ePrevPressed = eNow;

		// ---- Flashlight toggle (F) ----
		bool fNow = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
		if (fNow && !fPrevPressed) {
			if (player.inventory.has(ItemType::Flashlight))
				player.flashlightOn = !player.flashlightOn;
			else
				std::cout << "[Flashlight] You don't have a flashlight.\n";
		}
		fPrevPressed = fNow;

		// ---- Flashlight random cut-out ----
		if (player.flashlightOn && player.inventory.has(ItemType::Flashlight)) {
			player.flashlightFailAccum += dt;
			if (player.flashlightFailAccum >= 1.0f) {
				player.flashlightFailAccum -= 1.0f;
				float roll = (float)(rand() % 10000) / 10000.0f;
				if (roll < FLASHLIGHT_FAIL_CHANCE)
					player.flashlightOn = false;
			}
		}

		// ---- Sync player flashlight entity ----
		if (flashlightEntityIdx >= 0 && flashlightEntityIdx < (int)entities.size()) {
			Entity& fl = entities[flashlightEntityIdx];
			fl.pos    = player.pos;
			fl.active = player.flashlightOn && player.inventory.has(ItemType::Flashlight);
		}

		// ---- Tab: toggle minimap ----
		bool tabNow = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
		if (tabNow && !tabPrevPressed) {
			hud.minimapMode = (hud.minimapMode + 1) % 4;
			hud.showMinimap = (hud.minimapMode > 0);
		}
		tabPrevPressed = tabNow;

		// ---- I: toggle inventory screen ----
		bool iNow = (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS);
		if (iNow && !iPrevPressed) {
			hud.showInventoryScreen = !hud.showInventoryScreen;
			if (hud.showInventoryScreen) hud.showJournalScreen = false;
		}
		iPrevPressed = iNow;

		// ---- J: toggle journal screen ----
		bool jNow = (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS);
		if (jNow && !jPrevPressed) {
			hud.showJournalScreen = !hud.showJournalScreen;
			if (hud.showJournalScreen) hud.showInventoryScreen = false;
		}
		jPrevPressed = jNow;

		// ---- Arrow keys + scroll wheel: navigate inventory or journal ----
		if (hud.showInventoryScreen) {
			bool upNow   = (glfwGetKey(window, GLFW_KEY_UP)   == GLFW_PRESS);
			bool downNow = (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
			if (upNow   && !upPrevPressed)
				hud.inventorySelected = std::max(0, hud.inventorySelected - 1);
			if (downNow && !downPrevPressed)
				hud.inventorySelected = std::min(player.inventory.count() - 1, hud.inventorySelected + 1);
			upPrevPressed = upNow; downPrevPressed = downNow;
			if (scrollDelta != 0) {
				hud.inventorySelected = std::clamp(hud.inventorySelected + scrollDelta,
					0, std::max(0, player.inventory.count() - 1));
				scrollDelta = 0;
			}
		}
		else if (hud.showJournalScreen) {
			bool upNow   = (glfwGetKey(window, GLFW_KEY_UP)   == GLFW_PRESS);
			bool downNow = (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
			if (upNow   && !upPrevPressed)
				hud.journalSelected = std::max(0, hud.journalSelected - 1);
			if (downNow && !downPrevPressed)
				hud.journalSelected = std::min(player.journal.count() - 1, hud.journalSelected + 1);
			upPrevPressed = upNow; downPrevPressed = downNow;
			if (scrollDelta != 0) {
				hud.journalSelected = std::clamp(hud.journalSelected + scrollDelta,
					0, std::max(0, player.journal.count() - 1));
				scrollDelta = 0;
			}
		}
		else {
			scrollDelta = 0;
		}

		// ---- Animation & lighting updates ----
		updateEntities(entities, dt);
		for (auto& e : entities)
			updateEntityFlicker(e, dt);

		renderer.renderWorld(player, entities, textures);

		hud.drawCrosshair();
		hud.drawBars(player.health, player.maxHealth, player.stamina, player.maxStamina);
		if (hud.showMinimap) {
			hud.drawMinimap(player);
			hud.drawMinimapEntities(entities, player);
		}

		std::string prompt = buildPromptText(player, entities);
		if (!prompt.empty()) hud.drawPrompt(prompt);
		hud.drawInventory(player);
		hud.drawNotifications(dt);
		if (hud.showInventoryScreen) hud.drawInventoryScreen(player);
		if (hud.showJournalScreen)   hud.drawJournalScreen(player);
		renderer.present();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
