#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "Player.h"
#include "Texture.h"
#include "Entity.h"
#include "Map.h"

// ---------------------------------------------------------------------------
// TextureRegistry
// Maps TileType -> wall texture, EntityType -> sprite texture.
// Add textures at startup; renderer looks them up by key.
// ---------------------------------------------------------------------------
struct TextureRegistry {
	// Wall textures keyed by TileType
	std::unordered_map<uint8_t, CPUTexture> wallTextures;

	// Floor / ceiling textures (single for now; extend later)
	CPUTexture floorTex;
	CPUTexture ceilingTex;

	// Entity / sprite textures keyed by EntityType
	std::unordered_map<int, CPUTexture> entityTextures;

	// Convenience setters
	void setWall(TileType t, CPUTexture tex) {
		wallTextures[static_cast<uint8_t>(t)] = std::move(tex);
	}
	void setEntity(EntityType t, CPUTexture tex) {
		entityTextures[static_cast<int>(t)] = std::move(tex);
	}

	// Returns fallback (first wall texture) if key not found
	const CPUTexture* getWall(TileType t) const {
		auto it = wallTextures.find(static_cast<uint8_t>(t));
		if (it != wallTextures.end()) return &it->second;
		// fallback to WallStone
		auto fb = wallTextures.find(static_cast<uint8_t>(TileType::WallStone));
		return (fb != wallTextures.end()) ? &fb->second : nullptr;
	}
	const CPUTexture* getEntity(EntityType t) const {
		auto it = entityTextures.find(static_cast<int>(t));
		return (it != entityTextures.end()) ? &it->second : nullptr;
	}
};

// ---------------------------------------------------------------------------
// Renderer
// ---------------------------------------------------------------------------
class Renderer {
	std::vector<unsigned char> framebuffer;
	unsigned int fbTexture = 0;
	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
	unsigned int screenShader = 0;
	int screenW = 0;
	int screenH = 0;

public:
	Renderer(int w, int h, const std::string& vertPath, const std::string& fragPath);

	void renderWorld(const Player& player,
		const std::vector<Entity>& entities,
		const TextureRegistry& textures);

	void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void drawRect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
	void present();

	int width()  const { return screenW; }
	int height() const { return screenH; }
};