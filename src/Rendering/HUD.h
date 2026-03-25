#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Renderer.h"
#include "Player.h"
#include "Entity.h"
#include <deque>
#include <algorithm>

class HUD {
	Renderer& renderer;
	int screenW, screenH;


public:
	bool showMinimap = false;
	int  minimapMode = 0;  // 0=off, 1=close, 2=medium, 3=full map
	bool showInventoryScreen = false;
	bool showJournalScreen = false;
	int  inventorySelected = 0;
	int  journalSelected = 0;
	//
	struct Notification {
		std::string text;
		float timeLeft;  // seconds remaining
	};
	std::deque<Notification> notifications;
	static constexpr float NOTIF_DURATION = 3.0f;
	static constexpr int   NOTIF_MAX = 4;

	HUD(Renderer& r, int w, int h);
	void drawCrosshair();
	void drawBars(int health, int maxHealth, float stamina, float maxStamina);
	void drawMinimap(const Player& player, int tileSize = 2);
	void drawPrompt(const std::string& text);
	void drawInventory(const Player& player);
	void drawMinimapEntities(const std::vector<Entity>& entities,
		const Player& player, int tileSize = 2);
	void drawText(const std::string& text, int x, int y,
		unsigned char r, unsigned char g, unsigned char b);
	void pushNotification(const std::string& text);
	void drawNotifications(float dt);
	void drawInventoryScreen(const Player& player);
	void drawJournalScreen(const Player& player);
};