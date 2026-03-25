#include "Renderer.h"
#include "Shader.h"
#include "Raycaster.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <unordered_map>
#include <cmath>

Renderer::Renderer(int w, int h, const std::string& vertPath, const std::string& fragPath)
	: screenW(w), screenH(h)
{
	screenShader = makeShader(vertPath.c_str(), fragPath.c_str());
	framebuffer.resize(screenW * screenH * 3, 0);
	zBuffer.resize(screenW, 0.0f);

	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenW, screenH, 0,
		GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float quadVerts[] = {
		-1,-1, 0,1,   1,-1, 1,1,   1, 1, 1,0,
		-1,-1, 0,1,   1, 1, 1,0,  -1, 1, 0,0,
	};
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void Renderer::present()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenW, screenH,
		GL_RGB, GL_UNSIGNED_BYTE, framebuffer.data());
	glUseProgram(screenShader);
	glUniform1i(glGetUniformLocation(screenShader, "uScreen"), 0);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	if (x < 0 || x >= screenW || y < 0 || y >= screenH) return;
	int idx = (y * screenW + x) * 3;
	framebuffer[idx] = r;
	framebuffer[idx + 1] = g;
	framebuffer[idx + 2] = b;
}

void Renderer::drawRect(int x, int y, int w, int h,
	unsigned char r, unsigned char g, unsigned char b)
{
	for (int row = y; row < y + h; ++row)
		for (int col = x; col < x + w; ++col)
			setPixel(col, row, r, g, b);
}

void Renderer::renderWorld(const Player& player,
	const std::vector<Entity>& entities,
	const TextureRegistry& textures)
{
	const float fov = glm::radians(66.0f);
	const float halfTan = tanf(fov * 0.5f);
	const float projPlane = screenH * 0.5f;

	// Fast byte clamp: avoids glm::clamp overhead per pixel
	auto toByte = [](float f) -> unsigned char {
		int i = (int)(f * 255.0f);
		return (unsigned char)(i < 0 ? 0 : i > 255 ? 255 : i);
		};

	// Fast fmod-like UV wrap for positive values near 0-1
	auto wrapUV = [](float f) -> float {
		f -= (int)f;
		return f < 0.0f ? f + 1.0f : f;
		};

	std::unordered_map<uint32_t, const Entity*> decalMap;
	for (const auto& e : entities) {
		if (!e.active || e.type != PropType::Note) continue;
		if (!hasFlag(e.flags, PropFlags::WallDecal)) continue;
		uint32_t key = (uint32_t)(e.wallCell.x & 0xFFFF)
			| ((uint32_t)(e.wallCell.y & 0xFFFF) << 16);
		decalMap[key] = &e;
	}

	frameLights.clear();
	for (const auto& e : entities) {
		if (!e.active || e.lightRadius <= 0.0f) continue;
		float effectiveIntensity = e.lightIntensity * e.flickerValue;
		if (effectiveIntensity <= 0.0f) continue;
		frameLights.push_back({ e.pos, e.lightRadius, effectiveIntensity, e.lightColor });
	}

	auto sampleLights = [&](glm::vec2 wPos) -> glm::vec3 {
		glm::vec3 acc = { 0.0f, 0.0f, 0.0f };
		for (const auto& l : frameLights) {
			float c = lightContrib(wPos, l);
			if (c <= 0.0f) continue;               // outside radius, skip LOS
			//if (!hasLOS(l.pos, wPos)) continue;    // blocked by wall
			acc += l.color * c;
		}
		return acc;
		};

	for (int col = 0; col < screenW; ++col)
	{
		float camX = 2.0f * col / screenW - 1.0f;
		float rayAngle = player.angle + atanf(camX * halfTan);
		RayHit hit = castRay(player.pos, rayAngle);
		glm::vec2 rayDir = { cosf(rayAngle), sinf(rayAngle) };

		float cosAngle = cosf(rayAngle - player.angle);
		float invCosAngle = 1.0f / (cosAngle + 1e-7f);  // precompute reciprocal once
		float perpDist = hit.dist * cosAngle;
		perpDist = std::max(perpDist, 0.0001f);

		// Z-buffer uses the glass pane distance (primary hit) so sprites
		// standing behind glass are correctly depth-culled.
		zBuffer[col] = perpDist;

		float shade = glm::clamp(1.0f / (perpDist * 1.5f), 0.0f, 1.0f);
		if (hit.hitNS) shade *= 0.6f;

		int wallBottom = (int)(screenH * 0.5f + player.pitch + (EYE_HEIGHT / perpDist) * projPlane);
		int wallTop = (int)(screenH * 0.5f + player.pitch - ((WALL_HEIGHT - EYE_HEIGHT) / perpDist) * projPlane);
		int wallPx = std::max(1, wallBottom - wallTop);

		uint32_t dkey = (uint32_t)(hit.wallX & 0xFFFF)
			| ((uint32_t)(hit.wallY & 0xFFFF) << 16);
		auto dit = decalMap.find(dkey);
		const Entity* decal = (dit != decalMap.end()) ? dit->second : nullptr;

		const CPUTexture* wallTex = textures.getWall(hit.tile);

		const glm::vec2 wallWorldPos = hit.hitNS
			? glm::vec2{ hit.wallX + hit.hitPt, (float)hit.wallY }
		: glm::vec2{ (float)hit.wallX,       hit.wallY + hit.hitPt };

		const glm::vec2 wallLightPos = hit.hitNS
			? glm::vec2{ hit.wallX + hit.hitPt, hit.wallY + 0.05f * (player.pos.y < hit.wallY + 0.5f ? -1.0f : 1.0f) }
		: glm::vec2{ hit.wallX + 0.05f * (player.pos.x < hit.wallX + 0.5f ? -1.0f : 1.0f), hit.wallY + hit.hitPt };

		// --- Glass secondary hit: precompute column constants ONCE ---
		// These are used inside the row loop; computing them per-row was wasteful.
		float behindPerpDist = 0.0f, behindShade = 0.0f;
		int   behindTop2 = 0, behindPx2 = 1;
		const CPUTexture* behindTex = nullptr;
		if (hit.hasSecondary && isGlass(hit.tile))
		{
			behindPerpDist = hit.secondaryDist * cosAngle;  // cosAngle already computed
			behindShade = glm::clamp(1.0f / (behindPerpDist * 1.5f), 0.0f, 1.0f)
				* (hit.secondaryNS ? 0.6f : 1.0f);
			int behindBottom2 = (int)(screenH * 0.5f + player.pitch
				+ (EYE_HEIGHT / behindPerpDist) * projPlane);
			behindTop2 = (int)(screenH * 0.5f + player.pitch
				- ((WALL_HEIGHT - EYE_HEIGHT) / behindPerpDist) * projPlane);
			behindPx2 = std::max(1, behindBottom2 - behindTop2);
			behindTex = textures.getWall(hit.secondaryTile);
		}

		const glm::vec3 wallLight = sampleLights(wallWorldPos);

		for (int row = 0; row < screenH; ++row)
		{
			int idx = (row * screenW + col) * 3;
			glm::vec3 color;

			if (row < wallTop)
			{
				float denom = (screenH * 0.5f + player.pitch) - row;
				float rowDist = ((WALL_HEIGHT - EYE_HEIGHT) * projPlane) / (denom + 0.0001f);
				glm::vec2 cPos = player.pos + (rowDist * invCosAngle) * rayDir;
				float u = wrapUV(cPos.x);
				float v = wrapUV(cPos.y);
				float cShade = glm::clamp(1.0f / (rowDist * 1.5f), 0.0f, 1.0f);
				color = textures.ceilingTex.sample(u, v) * cShade * 0.2f;
			}
			else if (row >= wallBottom)
			{
				float denom = row - (screenH * 0.5f + player.pitch);
				float rowDist = (EYE_HEIGHT * projPlane) / (denom + 0.0001f);
				glm::vec2 fPos = player.pos + (rowDist * invCosAngle) * rayDir;
				float u = wrapUV(fPos.x);
				float v = wrapUV(fPos.y);
				float fShade = glm::clamp(1.0f / (rowDist * 1.5f), 0.0f, 1.0f);
				glm::vec3 fTex = textures.floorTex.sample(u, v);
				float floorLightFade = glm::clamp(1.0f - rowDist / 8.0f, 0.0f, 1.0f);
				color = fTex * fShade + wallLight * fTex * floorLightFade * 0.6f;
			}
			else
			{
				// Wall strip
				float wallV = (float)(row - wallTop) / (float)wallPx;

				constexpr float decalMin = 0.35f, decalMax = 0.65f;
				if (decal
					&& hit.hitPt >= decalMin && hit.hitPt <= decalMax
					&& wallV >= decalMin && wallV <= decalMax)
				{
					const CPUTexture* dTex = textures.getSpriteFrame0(PropType::Note);
					if (dTex) {
						float du = (hit.hitPt - decalMin) / (decalMax - decalMin);
						float dv = (wallV - decalMin) / (decalMax - decalMin);
						glm::vec3 dc = dTex->sample(du, dv);
						float alpha = dTex->hasAlpha()
							? dTex->sampleAlpha(du, dv)
							: dTex->luminanceAlpha(du, dv);
						color = (alpha > 0.1f)
							? dc * shade
							: (wallTex ? wallTex->sample(hit.hitPt, wallV) * shade : glm::vec3(0));
					}
					else {
						color = wallTex ? wallTex->sample(hit.hitPt, wallV) * shade : glm::vec3(0);
					}
				}
				else {
					glm::vec3 texColor = wallTex
						? wallTex->sample(hit.hitPt, wallV)
						: glm::vec3(0.5f);

					if (hit.hasSecondary && isGlass(hit.tile))
					{
						// All column constants precomputed above the row loop.
						const float behindV = glm::clamp(
							(float)(row - behindTop2) / (float)behindPx2, 0.0f, 1.0f);

						glm::vec3 behindColor = behindTex
							? behindTex->sample(hit.secondaryHitPt, behindV) * behindShade
							+ wallLight * behindTex->sample(hit.secondaryHitPt, behindV)
							: glm::vec3(0.2f);

						constexpr float GLASS_ALPHA = 0.80f;
						float glassAlpha = (texColor.r < 0.05f && texColor.g < 0.05f && texColor.b < 0.05f)
							? 0.0f : GLASS_ALPHA;
						glm::vec3 glassColor = texColor * shade + wallLight * texColor;
						color = glm::mix(behindColor, glassColor, glassAlpha);
					}
					else
					{
						color = texColor * shade + wallLight * texColor;
					}
				}
			}

			framebuffer[idx] = toByte(color.r);
			framebuffer[idx + 1] = toByte(color.g);
			framebuffer[idx + 2] = toByte(color.b);
		}
	}

	renderSprites(player, entities, textures);
}

void Renderer::renderSprites(const Player& player,
	const std::vector<Entity>& entities,
	const TextureRegistry& textures)
{
	const float fov = glm::radians(66.0f);
	const float halfTan = tanf(fov * 0.5f);
	const float projPlane = screenH * 0.5f;

	const float camDirX = cosf(player.angle);
	const float camDirY = sinf(player.angle);
	const float camPlaneX = -camDirY * halfTan;
	const float camPlaneY = camDirX * halfTan;
	const float invDet = 1.0f / (camPlaneX * camDirY - camDirX * camPlaneY);

	constexpr float SPRITE_CULL_DIST = 20.0f;

	struct SpriteEntry { float dist; const Entity* e; };
	std::vector<SpriteEntry> visible;
	visible.reserve(entities.size());

	for (const auto& e : entities)
	{
		if (!e.active) continue;
		if (hasFlag(e.flags, PropFlags::WallDecal)) continue;
		const CPUTexture* tex = textures.getSprite(e);
		if (!tex || tex->empty()) continue;

		float dx = e.pos.x - player.pos.x;
		float dy = e.pos.y - player.pos.y;
		float dist = sqrtf(dx * dx + dy * dy);
		if (dist > SPRITE_CULL_DIST) continue;

		visible.push_back({ dist, &e });
	}

	std::sort(visible.begin(), visible.end(),
		[](const SpriteEntry& a, const SpriteEntry& b) { return a.dist > b.dist; });

	for (const auto& se : visible)
	{
		const Entity& e = *se.e;
		const CPUTexture* tex = textures.getSprite(e);
		if (!tex || tex->empty()) continue;

		float dx = e.pos.x - player.pos.x;
		float dy = e.pos.y - player.pos.y;
		float transformX = invDet * (camDirY * dx - camDirX * dy);
		float transformY = invDet * (-camPlaneY * dx + camPlaneX * dy);

		if (transformY <= 0.01f) continue;

		int spriteScreenX = (int)(screenW * 0.5f * (1.0f + transformX / transformY));

		int drawBottom = (int)(screenH * 0.5f + player.pitch + (EYE_HEIGHT - e.worldZ) / transformY * projPlane);
		int drawTop = (int)(screenH * 0.5f + player.pitch - (e.worldZ + e.spriteWorldH - EYE_HEIGHT) / transformY * projPlane);
		int spriteH = std::max(1, drawBottom - drawTop);

		float aspect = (tex->width > 0 && tex->height > 0)
			? (float)tex->width / (float)tex->height : 1.0f;
		int spriteW = std::max(1, (int)(spriteH * aspect));
		int drawLeft = spriteScreenX - spriteW / 2;
		int drawRight = drawLeft + spriteW;

		if (drawRight < 0 || drawLeft >= screenW) continue;

		float shade = glm::clamp(1.0f / (transformY * 1.5f), 0.0f, 1.0f);

		// Light contribution computed once per sprite, not per pixel
		glm::vec3 lightAdd = { 0.0f, 0.0f, 0.0f };
		for (const auto& l : frameLights)
			lightAdd += l.color * lightContrib(e.pos, l);

		for (int col = std::max(0, drawLeft); col < std::min(screenW, drawRight); ++col)
		{
			if (transformY >= zBuffer[col]) continue;

			float u = glm::clamp((float)(col - drawLeft) / (float)(drawRight - drawLeft), 0.0f, 1.0f);
			if (e.mirrorX) u = 1.0f - u;

			for (int row = std::max(0, drawTop); row < std::min(screenH, drawBottom); ++row)
			{
				float v = glm::clamp((float)(row - drawTop) / (float)spriteH, 0.0f, 1.0f);

				float alpha = tex->hasAlpha()
					? tex->sampleAlpha(u, v)
					: tex->luminanceAlpha(u, v);
				if (alpha < 0.1f) continue;

				glm::vec3 texColor = tex->sample(u, v);
				glm::vec3 c = texColor * shade + lightAdd * texColor;

				setPixel(col, row,
					(unsigned char)(glm::clamp(c.r, 0.0f, 1.0f) * 255),
					(unsigned char)(glm::clamp(c.g, 0.0f, 1.0f) * 255),
					(unsigned char)(glm::clamp(c.b, 0.0f, 1.0f) * 255));
			}
		}
	}
}