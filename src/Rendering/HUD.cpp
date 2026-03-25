#include "Renderer.h"
#include "HUD.h"
#include "Map.h"
#include <vector>
#include <cmath>
#include <algorithm>

HUD::HUD(Renderer& r, int w, int h) : renderer(r), screenW(w), screenH(h) {}

void HUD::drawCrosshair()
{
	int leng = 10;
	for (int i = -leng; i <= leng; i++) {
		for (int j = -1; j <= 1; j++) {
			renderer.setPixel((screenW / 2) + i, (screenH / 2) + j, 155, 155, 155);
			renderer.setPixel((screenW / 2) + j, (screenH / 2) + i, 155, 155, 155);
		}
	}
}

void HUD::drawBars(int health, int maxHealth, float stamina, float maxStamina)
{
	float HP = glm::clamp((float)health / (float)maxHealth, 0.0f, 1.0f);
	float ST = glm::clamp(stamina / maxStamina, 0.0f, 1.0f);

	renderer.drawRect(10, 10, 250, 25, 100, 100, 100);
	renderer.drawRect(10, 40, 250, 25, 100, 100, 100);
	renderer.drawRect(14, 13, (int)(242 * HP), 19, 255, 0, 0);
	renderer.drawRect(14, 42, (int)(242 * ST), 19, 0, 255, 0);
}

// ---------------------------------------------------------------------------
// Minimap -- full overhead view, top-right corner.
// Color key:
//   Empty        = dark grey  (30,30,30)
//   WallStone    = mid grey   (120,120,120)
//   WallBrick    = brown      (140,80,40)
//   WallConcrete = blue-grey  (80,100,120)
//   WallSealed   = purple     (120,60,160)
//   DoorClosed*  = yellow     (220,200,0)
//   DoorLocked   = orange     (220,100,0)
//   DoorOpen*    = bright yel (255,255,100)
//   Other solid  = white      (200,200,200)
//   Player       = red dot    (255,50,50) + direction spike
// ---------------------------------------------------------------------------
void HUD::drawMinimap(const Player& player, int /*unused*/)
{
	// Mode 1: close follow  (VIEW_TILES=12, tileSize=4)
	// Mode 2: medium follow (VIEW_TILES=28, tileSize=2)
	// Mode 3: full map      (tileSize calculated to fit screenW/2)
	int VIEW_TILES, tileSize;
	bool fullMap = (minimapMode == 3);

	if (minimapMode == 1) { VIEW_TILES = 12; tileSize = 8; }
	else if (minimapMode == 2) { VIEW_TILES = 24; tileSize = 4; }
	else { VIEW_TILES = 0;  tileSize = 1; } // computed below

	int mapPxW, mapPxH, ox, oy;

	if (fullMap) {
		// Fit the whole map into at most half the screen width
		int maxPx = screenW / 2;
		tileSize = std::max(1, maxPx / std::max(MAP_W, 1));
		mapPxW = MAP_W * tileSize;
		mapPxH = MAP_H * tileSize;
		ox = screenW - mapPxW - 4;
		oy = 4;
	}
	else {
		int VIEW_W = VIEW_TILES * 2 + 1;
		int VIEW_H = VIEW_TILES * 2 + 1;
		mapPxW = VIEW_W * tileSize;
		mapPxH = VIEW_H * tileSize;
		ox = screenW - mapPxW - 4;
		oy = 4;
	}

	// Border
	for (int x = ox - 1; x <= ox + mapPxW; ++x) {
		renderer.setPixel(x, oy - 1, 0, 0, 0);
		renderer.setPixel(x, oy + mapPxH, 0, 0, 0);
	}
	for (int y = oy - 1; y <= oy + mapPxH; ++y) {
		renderer.setPixel(ox - 1, y, 0, 0, 0);
		renderer.setPixel(ox + mapPxW, y, 0, 0, 0);
	}

	// Draw tiles
	int pcx = (int)player.pos.x;
	int pcy = (int)player.pos.y;

	if (fullMap) {
		for (int my = 0; my < MAP_H; ++my) {
			for (int mx = 0; mx < MAP_W; ++mx) {
				TileType t = getTile(mx, my);
				unsigned char r, g, b;
				switch (t) {
				case TileType::Empty:          r = 30;  g = 30;  b = 30;  break;
				case TileType::WallStone:      r = 120; g = 120; b = 120; break;
				case TileType::WallBrick:      r = 140; g = 80;  b = 40;  break;
				case TileType::WallConcrete:   r = 80;  g = 100; b = 120; break;
				case TileType::WallSealed:     r = 120; g = 60;  b = 160; break;
				case TileType::DoorClosedH:
				case TileType::DoorClosedV:    r = 220; g = 200; b = 0;   break;
				case TileType::DoorLocked:     r = 220; g = 100; b = 0;   break;
				case TileType::DoorOpenH:
				case TileType::DoorOpenV:      r = 255; g = 255; b = 100; break;
				case TileType::ZoneExit:
				case TileType::ZoneTransition: r = 0;   g = 255; b = 180; break;
				default:
					if (isSolid(t)) { r = 200; g = 200; b = 200; }
					else { r = 20;  g = 20;  b = 40; }
					break;
				}
				for (int py2 = 0; py2 < tileSize; ++py2)
					for (int px2 = 0; px2 < tileSize; ++px2)
						renderer.setPixel(ox + mx * tileSize + px2,
							oy + my * tileSize + py2, r, g, b);
			}
		}
		// Player dot — bright red, 3x3
		int cx = ox + pcx * tileSize + tileSize / 2;
		int cy = oy + pcy * tileSize + tileSize / 2;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx)
				renderer.setPixel(cx + dx, cy + dy, 255, 50, 50);
		for (int s = 1; s <= 4; ++s)
			renderer.setPixel(cx + (int)(cosf(player.angle) * s),
				cy + (int)(sinf(player.angle) * s), 255, 200, 200);
	}
	else {
		int VIEW_W = VIEW_TILES * 2 + 1;
		int VIEW_H = VIEW_TILES * 2 + 1;
		for (int ty = 0; ty < VIEW_H; ++ty) {
			for (int tx = 0; tx < VIEW_W; ++tx) {
				int mx = pcx - VIEW_TILES + tx;
				int my = pcy - VIEW_TILES + ty;
				TileType t = getTile(mx, my);
				unsigned char r, g, b;
				switch (t) {
				case TileType::Empty:          r = 30;  g = 30;  b = 30;  break;
				case TileType::WallStone:      r = 120; g = 120; b = 120; break;
				case TileType::WallBrick:      r = 140; g = 80;  b = 40;  break;
				case TileType::WallConcrete:   r = 80;  g = 100; b = 120; break;
				case TileType::WallSealed:     r = 120; g = 60;  b = 160; break;
				case TileType::DoorClosedH:
				case TileType::DoorClosedV:    r = 220; g = 200; b = 0;   break;
				case TileType::DoorLocked:     r = 220; g = 100; b = 0;   break;
				case TileType::DoorOpenH:
				case TileType::DoorOpenV:      r = 255; g = 255; b = 100; break;
				case TileType::ZoneExit:
				case TileType::ZoneTransition: r = 0;   g = 255; b = 180; break;
				default:
					if (isSolid(t)) { r = 200; g = 200; b = 200; }
					else { r = 20;  g = 20;  b = 40; }
					break;
				}
				for (int py2 = 0; py2 < tileSize; ++py2)
					for (int px2 = 0; px2 < tileSize; ++px2)
						renderer.setPixel(ox + tx * tileSize + px2,
							oy + ty * tileSize + py2, r, g, b);
			}
		}
		// Player dot
		int cx = ox + VIEW_TILES * tileSize + (int)((player.pos.x - pcx) * tileSize);
		int cy = oy + VIEW_TILES * tileSize + (int)((player.pos.y - pcy) * tileSize);
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx)
				renderer.setPixel(cx + dx, cy + dy, 255, 50, 50);
		for (int s = 1; s <= 5; ++s)
			renderer.setPixel(cx + (int)(cosf(player.angle) * s),
				cy + (int)(sinf(player.angle) * s), 255, 200, 200);
	}
}
static const uint8_t MINI_FONT[][5] = {
	// Space
	{0b000,0b000,0b000,0b000,0b000},
	// A-Z
	{0b010,0b101,0b111,0b101,0b101}, // A
	{0b110,0b101,0b110,0b101,0b110}, // B
	{0b011,0b100,0b100,0b100,0b011}, // C
	{0b110,0b101,0b101,0b101,0b110}, // D
	{0b111,0b100,0b110,0b100,0b111}, // E
	{0b111,0b100,0b110,0b100,0b100}, // F
	{0b011,0b100,0b101,0b101,0b011}, // G
	{0b101,0b101,0b111,0b101,0b101}, // H
	{0b111,0b010,0b010,0b010,0b111}, // I
	{0b111,0b001,0b001,0b101,0b010}, // J
	{0b101,0b110,0b100,0b110,0b101}, // K
	{0b100,0b100,0b100,0b100,0b111}, // L
	{0b101,0b111,0b111,0b101,0b101}, // M
	{0b101,0b111,0b111,0b111,0b101}, // N
	{0b010,0b101,0b101,0b101,0b010}, // O
	{0b110,0b101,0b110,0b100,0b100}, // P
	{0b010,0b101,0b101,0b111,0b011}, // Q
	{0b110,0b101,0b110,0b101,0b101}, // R
	{0b011,0b100,0b010,0b001,0b110}, // S
	{0b111,0b010,0b010,0b010,0b010}, // T
	{0b101,0b101,0b101,0b101,0b111}, // U
	{0b101,0b101,0b101,0b101,0b010}, // V
	{0b101,0b101,0b111,0b111,0b101}, // W
	{0b101,0b101,0b010,0b101,0b101}, // X
	{0b101,0b101,0b010,0b010,0b010}, // Y
	{0b111,0b001,0b010,0b100,0b111}, // Z
	// 0-9
	{0b010,0b101,0b101,0b101,0b010}, // 0
	{0b010,0b110,0b010,0b010,0b111}, // 1
	{0b110,0b001,0b010,0b100,0b111}, // 2
	{0b110,0b001,0b010,0b001,0b110}, // 3
	{0b101,0b101,0b111,0b001,0b001}, // 4
	{0b111,0b100,0b110,0b001,0b110}, // 5
	{0b010,0b100,0b110,0b101,0b010}, // 6
	{0b111,0b001,0b010,0b010,0b010}, // 7
	{0b010,0b101,0b010,0b101,0b010}, // 8
	{0b010,0b101,0b011,0b001,0b010}, // 9
};

static int charToFontIndex(char c) {
	if (c == ' ') return 0;
	if (c >= 'A' && c <= 'Z') return 1 + (c - 'A');
	if (c >= 'a' && c <= 'z') return 1 + (c - 'a'); // lowercase maps to uppercase
	if (c >= '0' && c <= '9') return 27 + (c - '0');
	return 0; // fallback: space
}

void HUD::drawPrompt(const std::string& text)
{
	if (text.empty()) return;

	const int CHAR_W = 4;  // 3px glyph + 1px gap
	const int CHAR_H = 5;
	const int PADDING = 4;
	const int BOX_H = CHAR_H + PADDING * 2;
	const int BOX_W = (int)text.size() * CHAR_W + PADDING * 2;

	// Position: centred horizontally, just below crosshair
	int boxX = (screenW - BOX_W) / 2;
	int boxY = screenH / 2 + 20;

	// Dark semi-transparent background (just a dark rect)
	renderer.drawRect(boxX, boxY, BOX_W, BOX_H, 10, 10, 10);

	// Draw each character
	int cx = boxX + PADDING;
	int cy = boxY + PADDING;
	for (char c : text) {
		int fi = charToFontIndex(c);
		for (int row = 0; row < CHAR_H; ++row) {
			uint8_t bits = MINI_FONT[fi][row];
			for (int bit = 2; bit >= 0; --bit) {
				if (bits & (1 << bit)) {
					renderer.setPixel(cx + (2 - bit), cy + row, 220, 220, 180);
				}
			}
		}
		cx += CHAR_W;
	}
}

void HUD::drawInventory(const Player& player)
{
	const int SLOT_SIZE = 24;
	const int SLOT_GAP = 3;
	const int SLOTS = Inventory::MAX_SLOTS;
	const int totalW = SLOTS * (SLOT_SIZE + SLOT_GAP) - SLOT_GAP;
	const int startX = (screenW - totalW) / 2;
	const int startY = screenH - SLOT_SIZE - 8;

	for (int i = 0; i < SLOTS; ++i) {
		int sx = startX + i * (SLOT_SIZE + SLOT_GAP);
		int sy = startY;

		// Slot border (dark gray)
		renderer.drawRect(sx, sy, SLOT_SIZE, SLOT_SIZE, 40, 40, 40);
		// Slot inner (slightly lighter)
		renderer.drawRect(sx + 1, sy + 1, SLOT_SIZE - 2, SLOT_SIZE - 2, 25, 25, 25);

		if (i < player.inventory.count()) {
			ItemType item = player.inventory.slots[i];

			// Item colour indicator
			unsigned char r = 180, g = 180, b = 180;
			switch (item) {
			case ItemType::Key:         r = 220; g = 190; b = 60;  break; // gold
			case ItemType::Flashlight:  r = 180; g = 220; b = 255; break; // light blue
			case ItemType::Baton:       r = 160; g = 120; b = 80;  break; // brown
			case ItemType::FirstAidKit: r = 220; g = 60;  b = 60;  break; // red
			case ItemType::Logbook:     r = 180; g = 160; b = 120; break; // tan
			default: break;
			}

			// Filled slot indicator (inner block)
			renderer.drawRect(sx + 4, sy + 4, SLOT_SIZE - 8, SLOT_SIZE - 12, r, g, b);

			// Item initial letter at bottom of slot
			const char* name = itemName(item);
			int fi = charToFontIndex(name[0]);
			int lx = sx + (SLOT_SIZE - 3) / 2;
			int ly = sy + SLOT_SIZE - 7;
			for (int row = 0; row < 5; ++row) {
				uint8_t bits = MINI_FONT[fi][row];
				for (int bit = 2; bit >= 0; --bit)
					if (bits & (1 << bit))
						renderer.setPixel(lx + (2 - bit), ly + row, 220, 220, 180);
			}
		}
	}
}

void HUD::drawMinimapEntities(const std::vector<Entity>& entities,
	const Player& player, int /*unused*/)
{
	if (minimapMode == 0) return;

	bool fullMap = (minimapMode == 3);
	int tileSize, VIEW_TILES, ox, oy;

	if (minimapMode == 1) { VIEW_TILES = 12; tileSize = 8; }
	else if (minimapMode == 2) { VIEW_TILES = 24; tileSize = 4; }
	else { VIEW_TILES = 0;  tileSize = std::max(1, (screenW / 2) / std::max(MAP_W, 1)); }

	int mapPxW;
	if (fullMap) {
		mapPxW = MAP_W * tileSize;
	}
	else {
		mapPxW = (VIEW_TILES * 2 + 1) * tileSize;
	}
	ox = screenW - mapPxW - 4;
	oy = 4;

	for (const auto& e : entities) {
		if (!e.active) continue;
		if (hasFlag(e.flags, PropFlags::WallDecal)) continue;

		float dx = e.pos.x - player.pos.x;
		float dy = e.pos.y - player.pos.y;

		int px, py;
		if (fullMap) {
			// Absolute position on full map
			px = ox + (int)(e.pos.x * tileSize) + tileSize / 2;
			py = oy + (int)(e.pos.y * tileSize) + tileSize / 2;
		}
		else {
			if (fabsf(dx) > VIEW_TILES || fabsf(dy) > VIEW_TILES) continue;
			px = ox + (int)((VIEW_TILES + dx) * tileSize);
			py = oy + (int)((VIEW_TILES + dy) * tileSize);
		}

		unsigned char r, g, b;
		auto v = static_cast<uint8_t>(e.type);
		
		if (v >= 60 && v <= 64) { r = 255; g = 220; b = 0; }
		else if (hasFlag(e.flags, PropFlags::Interactable)) { r = 0;   g = 220; b = 220; }
		else if (hasFlag(e.flags, PropFlags::Collideable)) { r = 100; g = 130; b = 180; }
		else { r = 120; g = 120; b = 120; }

		int rad = 0;
		for (int ddy = -rad; ddy <= rad; ++ddy)
			for (int ddx = -rad; ddx <= rad; ++ddx)
				renderer.setPixel(px + ddx, py + ddy, r, g, b);
	}
}
void HUD::drawText(const std::string& text, int x, int y,
	unsigned char r, unsigned char g, unsigned char b)
{
	const int CHAR_W = 4;
	const int CHAR_H = 5;
	int cx = x;
	for (char c : text) {
		int fi = charToFontIndex(c);
		for (int row = 0; row < CHAR_H; ++row) {
			uint8_t bits = MINI_FONT[fi][row];
			for (int bit = 2; bit >= 0; --bit)
				if (bits & (1 << bit))
					renderer.setPixel(cx + (2 - bit), y + row, r, g, b);
		}
		cx += CHAR_W;
	}
}

void HUD::pushNotification(const std::string& text)
{
	if ((int)notifications.size() >= NOTIF_MAX)
		notifications.pop_front();
	notifications.push_back({ text, NOTIF_DURATION });
}

// Call once per frame in the render section. Draws and ages notifications.
void HUD::drawNotifications(float dt)
{
	constexpr int PAD = 10;
	constexpr int LINE_H = 12;
	// Bars are at y=10 (HP, 25px tall) and y=40 (stamina, 25px tall)
	// Start just below them with a small gap
	int y = 75;

	for (auto it = notifications.begin(); it != notifications.end(); ) {
		it->timeLeft -= dt;
		if (it->timeLeft <= 0.0f) { it = notifications.erase(it); continue; }

		float fade = std::min(1.0f, it->timeLeft / 0.5f);
		unsigned char fg = (unsigned char)(220 * fade);

		int w = (int)it->text.size() * 4 + PAD * 2;
		renderer.drawRect(PAD - 2, y - 2, w, LINE_H + 2, 0, 0, 0);
		drawText(it->text, PAD, y, fg, fg, (unsigned char)(fg * 0.7f));

		y += LINE_H + 4;
		++it;
	}
}

void HUD::drawInventoryScreen(const Player& player)
{
	const int SW = screenW;
	const int SH = screenH;

	for (int oy = 0; oy < SH; oy += 2)
		renderer.drawRect(0, oy, SW, 1, 0, 0, 0);

	constexpr int PANEL_W = 240;
	constexpr int PANEL_H = 180;
	const int px = (SW - PANEL_W) / 2;
	const int py = (SH - PANEL_H) / 2;

	renderer.drawRect(px, py, PANEL_W, PANEL_H, 15, 15, 20);
	renderer.drawRect(px, py, PANEL_W, 1, 80, 80, 100);
	renderer.drawRect(px, py + PANEL_H - 1, PANEL_W, 1, 80, 80, 100);
	renderer.drawRect(px, py, 1, PANEL_H, 80, 80, 100);
	renderer.drawRect(px + PANEL_W - 1, py, 1, PANEL_H, 80, 80, 100);

	drawText("INVENTORY", px + 8, py + 8, 200, 200, 200);
	renderer.drawRect(px + 8, py + 18, PANEL_W - 16, 1, 60, 60, 70);

	const int COUNT = player.inventory.count();
	if (COUNT == 0) {
		drawText("(empty)", px + 16, py + 30, 90, 90, 90);
	}
	else {
		for (int i = 0; i < COUNT; ++i) {
			int iy = py + 28 + i * 16;
			bool sel = (i == inventorySelected);
			if (sel)
				renderer.drawRect(px + 6, iy - 1, PANEL_W - 12, 14, 40, 40, 70);
			unsigned char lr = sel ? 255 : 150;
			unsigned char lg = sel ? 255 : 150;
			unsigned char lb = sel ? 255 : 150;
			std::string prefix = sel ? "> " : "  ";
			drawText(prefix + itemName(player.inventory.get(i)), px + 14, iy, lr, lg, lb);
		}

		if (inventorySelected < COUNT) {
			const char* desc = itemDescription(player.inventory.get(inventorySelected));
			int dy = py + PANEL_H - 28;
			renderer.drawRect(px + 6, dy - 3, PANEL_W - 12, 1, 60, 60, 70);
			drawText(desc, px + 10, dy + 2, 160, 160, 120);
		}
	}

	drawText("UP/DOWN SELECT   I CLOSE", px + 6, py + PANEL_H - 14, 60, 60, 70);
}
void HUD::drawJournalScreen(const Player& player)
{
	const int SW = screenW;
	const int SH = screenH;

	// Scanline overlay
	for (int oy = 0; oy < SH; oy += 2)
		renderer.drawRect(0, oy, SW, 1, 0, 0, 0);

	constexpr int PANEL_W = 320;
	constexpr int PANEL_H = 190;
	constexpr int LIST_W = 100;   // left column width
	const int px = (SW - PANEL_W) / 2;
	const int py = (SH - PANEL_H) / 2;

	// Background + border
	renderer.drawRect(px, py, PANEL_W, PANEL_H, 15, 12, 10);
	renderer.drawRect(px, py, PANEL_W, 1, 70, 55, 40);
	renderer.drawRect(px, py + PANEL_H - 1, PANEL_W, 1, 70, 55, 40);
	renderer.drawRect(px, py, 1, PANEL_H, 70, 55, 40);
	renderer.drawRect(px + PANEL_W - 1, py, 1, PANEL_H, 70, 55, 40);

	// Title bar
	drawText("NOTES", px + 8, py + 8, 190, 160, 100);
	renderer.drawRect(px + 8, py + 18, PANEL_W - 16, 1, 60, 50, 35);

	// Vertical divider between list and body
	renderer.drawRect(px + LIST_W, py + 20, 1, PANEL_H - 22, 50, 42, 30);

	const int COUNT = player.journal.count();

	if (COUNT == 0) {
		drawText("(no notes)", px + 6, py + 30, 70, 60, 50);
	}
	else {
		// ---- Left: entry titles ----
		constexpr int LIST_ITEM_H = 14;
		constexpr int LIST_START_Y = 24;
		constexpr int LIST_MAX_VISIBLE = 10;

		// Clamp selection
		int sel = journalSelected;

		for (int i = 0; i < COUNT && i < LIST_MAX_VISIBLE; ++i) {
			int iy = py + LIST_START_Y + i * LIST_ITEM_H;
			bool selected = (i == sel);

			if (selected)
				renderer.drawRect(px + 2, iy - 1, LIST_W - 4, LIST_ITEM_H, 50, 38, 22);

			unsigned char lr = selected ? 230 : 140;
			unsigned char lg = selected ? 190 : 115;
			unsigned char lb = selected ? 120 : 70;

			// Truncate title to ~12 chars to fit column
			const JournalEntry* entry = player.journal.get(i);
			std::string title = entry ? entry->title : "";
			if ((int)title.size() > 12) title = title.substr(0, 11) + ".";

			std::string prefix = selected ? ">" : " ";
			drawText(prefix + title, px + 4, iy, lr, lg, lb);
		}

		// ---- Right: body text of selected entry ----
		const JournalEntry* selEntry = player.journal.get(sel);
		if (selEntry) {
			const int TEXT_X = px + LIST_W + 6;
			const int TEXT_Y = py + 24;
			const int MAX_LINE_W = PANEL_W - LIST_W - 12;  // pixels
			const int CHARS_PER_LINE = MAX_LINE_W / 4;      // 4px per char
			const int LINE_H = 11;
			const int MAX_LINES = (PANEL_H - 36) / LINE_H;

			// Split body on '\n', then soft-wrap long lines
			const std::string& body = selEntry->body;
			std::vector<std::string> lines;
			std::string current;
			for (size_t ci = 0; ci < body.size(); ++ci) {
				char c = body[ci];
				if (c == '\n') {
					lines.push_back(current);
					current.clear();
				}
				else {
					current += c;
					if ((int)current.size() >= CHARS_PER_LINE) {
						lines.push_back(current);
						current.clear();
					}
				}
				if ((int)lines.size() >= MAX_LINES) break;
			}
			if (!current.empty() && (int)lines.size() < MAX_LINES)
				lines.push_back(current);

			for (int li = 0; li < (int)lines.size(); ++li)
				drawText(lines[li], TEXT_X, TEXT_Y + li * LINE_H, 180, 160, 120);
		}
	}

	drawText("UP/DOWN SELECT   J CLOSE", px + 6, py + PANEL_H - 14, 50, 42, 30);
}