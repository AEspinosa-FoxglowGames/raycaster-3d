#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Renderer.h"

class HUD {
	Renderer& renderer;
	int screenW, screenH;

public:
	HUD(Renderer& r, int w, int h);
	void drawCrosshair();
	void drawBars(int health,int maxHealth, float stamina, float maxStamina);
};