#pragma once
#include <glad/glad.h>
#include <string>
#include <iostream>
#include "Player.h"
#include "Texture.h"
#include <vector>
#include "Raycaster.h"
#include "Map.h"
#include "Entity.h"
#include <unordered_map>
#include "World.h"


class Renderer {
	std::vector<unsigned char> framebuffer;
	unsigned int fbTexture;
	unsigned int quadVAO, quadVBO;
	unsigned int screenShader;
	int screenW, screenH;

public:
	Renderer(int w, int h, const std::string& shaderVert, const std::string& shaderFrag);
	void renderWorld(const Player& player,
		const CPUTexture& wallTex,
		const CPUTexture& floorTex,
		const CPUTexture& ceilingTex,
		const std::vector<Entity>& entities,
		const std::unordered_map<EntityType, CPUTexture>& entityTextures);
	void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void present();
	void drawRect(int x,int y,int w, int h, unsigned char r, unsigned char g, unsigned char b);
};