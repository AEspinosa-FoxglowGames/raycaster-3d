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
// SpriteSheet
// A sequence of CPUTexture frames for animated or state-driven sprites.
// Single-frame sprites just have frames.size() == 1.
// ---------------------------------------------------------------------------
struct SpriteSheet {
	std::vector<CPUTexture> frames;
	float defaultFPS = 8.0f;

	bool empty() const { return frames.empty(); }

	// Returns the frame at index idx, wrapping if necessary.
	const CPUTexture* frame(int idx) const {
		if (frames.empty()) return nullptr;
		int n = (int)frames.size();
		return &frames[((idx % n) + n) % n];
	}
};

struct PointLight {
	glm::vec2 pos;
	float     radius;
	float     intensity;   // already flickered
	glm::vec3 color;
};
// Build a SpriteSheet from one or more image file paths.
// Single image: makeSpriteSheet({"assets/sprites/locker.png"})
// Animated:     makeSpriteSheet({"candle0.png","candle1.png","candle2.png"}, 10.0f)
inline SpriteSheet makeSpriteSheet(std::vector<std::string> paths, float fps = 8.0f)
{
	SpriteSheet s;
	s.defaultFPS = fps;
	for (auto& p : paths)
		s.frames.push_back(loadCPUTexture(p.c_str()));
	return s;
}

// ---------------------------------------------------------------------------
// TextureRegistry
// ---------------------------------------------------------------------------
struct TextureRegistry {
	// Wall textures keyed by TileType
	std::unordered_map<uint8_t, CPUTexture> wallTextures;

	// Floor / ceiling (single for now)
	CPUTexture floorTex;
	CPUTexture ceilingTex;

	// Sprite sheets keyed by PropType (replaces old entityTextures)
	std::unordered_map<uint8_t, SpriteSheet> spriteSheets;

	// --- Setters ---
	void setWall(TileType t, CPUTexture tex) {
		wallTextures[static_cast<uint8_t>(t)] = std::move(tex);
	}

	// Register a single-frame sprite.
	void setSprite(PropType t, CPUTexture tex) {
		SpriteSheet s;
		s.frames.push_back(std::move(tex));
		spriteSheets[static_cast<uint8_t>(t)] = std::move(s);
	}

	// Register an animated (or multi-state) sprite sheet.
	void setSpriteSheet(PropType t, SpriteSheet sheet) {
		spriteSheets[static_cast<uint8_t>(t)] = std::move(sheet);
	}

	// --- Getters ---
	const CPUTexture* getWall(TileType t) const {
		auto it = wallTextures.find(static_cast<uint8_t>(t));
		if (it != wallTextures.end()) return &it->second;
		auto fb = wallTextures.find(static_cast<uint8_t>(TileType::WallStone));
		return (fb != wallTextures.end()) ? &fb->second : nullptr;
	}

	// Returns the correct frame for entity e (uses e.currentFrame).
	// Returns nullptr if no sprite registered for this type.
	const CPUTexture* getSprite(const Entity& e) const {
		auto it = spriteSheets.find(static_cast<uint8_t>(e.type));
		if (it == spriteSheets.end()) return nullptr;
		return it->second.frame(e.currentFrame);
	}

	// Returns frame 0 (for wall decal lookups that don't use animation).
	const CPUTexture* getSpriteFrame0(PropType t) const {
		auto it = spriteSheets.find(static_cast<uint8_t>(t));
		if (it == spriteSheets.end()) return nullptr;
		return it->second.frame(0);
	}
};

// ---------------------------------------------------------------------------
// Renderer
// ---------------------------------------------------------------------------
class Renderer {
	std::vector<unsigned char> framebuffer;
	std::vector<float>         zBuffer;   // perpendicular distance per screen column
	unsigned int fbTexture = 0;
	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
	unsigned int screenShader = 0;
	int screenW = 0;
	int screenH = 0;
	std::vector<PointLight> frameLights;
	// Sprite pass -- called internally at the end of renderWorld.
	void renderSprites(const Player& player,
		const std::vector<Entity>& entities,
		const TextureRegistry& textures);
	static float lightContrib(glm::vec2 worldPos, const PointLight& l)
	{
		float dx = worldPos.x - l.pos.x;
		float dy = worldPos.y - l.pos.y;
		float dist2 = dx * dx + dy * dy;
		float r2 = l.radius * l.radius;
		if (dist2 >= r2) return 0.0f;
		// Smooth inverse-square falloff, clamped at 1.0
		float atten = 1.0f - (dist2 / r2);  // linear falloff envelope
		atten = atten * atten;               // square it for nicer roll-off
		return l.intensity * atten;
	}
public:
	// World scale constants (shared with renderSprites).
	static constexpr float WALL_HEIGHT = 4.0f;
	static constexpr float EYE_HEIGHT = 2.25f;

	Renderer(int w, int h, const std::string& vertPath, const std::string& fragPath);

	// Full world render: walls/floor/ceiling, then billboarded sprites.
	void renderWorld(const Player& player,
		const std::vector<Entity>& entities,
		const TextureRegistry& textures);

	void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void drawRect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
	void present();

	int width()  const { return screenW; }
	int height() const { return screenH; }
};