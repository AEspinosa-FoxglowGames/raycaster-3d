#include "Renderer.h"
#include "Shader.h"
#include "Raycaster.h"
#include <glm/glm.hpp>
#include <algorithm>

Renderer::Renderer(int w, int h, const std::string& vertPath, const std::string& fragPath)
	: screenW(w), screenH(h)
{
	screenShader = makeShader(vertPath.c_str(), fragPath.c_str());
	framebuffer.resize(screenW * screenH * 3, 0);

	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenW, screenH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
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
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenW, screenH, GL_RGB, GL_UNSIGNED_BYTE, framebuffer.data());
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

void Renderer::drawRect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b)
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
	const float halfTan = tanf(fov / 2.0f);

	for (int col = 0; col < screenW; ++col)
	{
		float camX = 2.0f * col / screenW - 1.0f;
		float rayAngle = player.angle + atanf(camX * halfTan);
		RayHit hit = castRay(player.pos, rayAngle);
		glm::vec2 rayDir = { cosf(rayAngle), sinf(rayAngle) };

		// Distance-based shading; NS faces slightly darker
		float shade = glm::clamp(1.0f / (hit.dist * 1.5f), 0.0f, 1.0f);
		if (hit.hitNS) shade *= 0.6f;

		int wallPx = (int)(screenH / hit.dist);
		int wallTop = screenH / 2 - wallPx / 2;
		int wallBottom = screenH / 2 + wallPx / 2;

		// Find wall-face decal entity for this column (O(n) -- fine for small N)
		const Entity* decal = nullptr;
		for (const auto& e : entities) {
			if (!e.active || !e.collected == false) continue; // skip inactive or already collected
			if (e.wallCell.x == hit.wallX && e.wallCell.y == hit.wallY) {
				decal = &e;
				break;
			}
		}

		// Texture for this wall tile
		const CPUTexture* wallTex = textures.getWall(hit.tile);

		// Door rendering: door + its side strips are drawn on the SAME recessed plane (handled in castRay).
		// To avoid angle-dependent gaps, we do NOT render separate jamb geometry. Instead:
		//   - centre region uses the door texture (remapped U)
		//   - side strips use the neighboring wall texture (so the frame blends into the wall)
		TileType doorBlendTile = TileType::WallStone;
		if (hit.isDoorHit) {
			TileType n = getTile(hit.wallX, hit.wallY - 1);
			TileType s = getTile(hit.wallX, hit.wallY + 1);
			TileType w = getTile(hit.wallX - 1, hit.wallY);
			TileType e = getTile(hit.wallX + 1, hit.wallY);
			if (isOpaque(n) && !isDoor(n)) doorBlendTile = n;
			else if (isOpaque(s) && !isDoor(s)) doorBlendTile = s;
			else if (isOpaque(w) && !isDoor(w)) doorBlendTile = w;
			else if (isOpaque(e) && !isDoor(e)) doorBlendTile = e;
			else doorBlendTile = TileType::WallStone;
		}
		const CPUTexture* doorBlendTex = hit.isDoorHit ? textures.getWall(doorBlendTile) : nullptr;
		const CPUTexture* doorTex = hit.isDoorHit ? textures.getWall(hit.tile) : nullptr;

		for (int row = 0; row < screenH; ++row)
		{
			int idx = (row * screenW + col) * 3;
			glm::vec3 color;

			if (row < wallTop)
			{
				// Ceiling
				float rowDist = (screenH / 2.0f) / (screenH / 2.0f - row + 0.0001f);
				glm::vec2 cPos = player.pos + rowDist * rayDir;
				float u = fmodf(cPos.x, 1.0f); if (u < 0) u += 1.0f;
				float v = fmodf(cPos.y, 1.0f); if (v < 0) v += 1.0f;
				float cShade = glm::clamp(1.0f / (rowDist * 1.5f), 0.0f, 1.0f);
				color = textures.ceilingTex.sample(u, v) * cShade * 0.2f;
			}
			else if (row >= wallBottom)
			{
				// Floor
				float rowDist = (screenH / 2.0f) / (row - screenH / 2.0f + 0.0001f);
				glm::vec2 fPos = player.pos + rowDist * rayDir;
				float u = fmodf(fPos.x, 1.0f); if (u < 0) u += 1.0f;
				float v = fmodf(fPos.y, 1.0f); if (v < 0) v += 1.0f;
				float fShade = glm::clamp(1.0f / (rowDist * 1.5f), 0.0f, 1.0f);
				color = textures.floorTex.sample(u, v) * fShade;
			}
			else
			{
				// Wall strip
				float wallV = (float)(row - wallTop) / (float)(wallPx);

				// Decal overlay (centre region of wall face)
				constexpr float decalMin = 0.35f, decalMax = 0.65f;
				if (decal
					&& hit.hitPt >= decalMin && hit.hitPt <= decalMax
					&& wallV >= decalMin && wallV <= decalMax)
				{
					const CPUTexture* dTex = textures.getEntity(decal->type);
					if (dTex) {
						float du = (hit.hitPt - decalMin) / (decalMax - decalMin);
						float dv = (wallV - decalMin) / (decalMax - decalMin);
						glm::vec3 dc = dTex->sample(du, dv);
						// Simple alpha: if the decal pixel is near-black treat as transparent
						color = (dc.r + dc.g + dc.b > 0.15f)
							? dc * shade
							: (wallTex ? wallTex->sample(hit.hitPt, wallV) * shade : glm::vec3(0));
					}
					else {
						color = wallTex ? wallTex->sample(hit.hitPt, wallV) * shade : glm::vec3(0);
					}
				}
				else {
					if (hit.isDoorHit) {
						// Frame strips: wall texture. Panel: door texture (U remapped).
						if (hit.hitPt >= DOOR_MIN && hit.hitPt <= DOOR_MAX) {
							float du = (hit.hitPt - DOOR_MIN) / (DOOR_MAX - DOOR_MIN);
							color = doorTex ? doorTex->sample(du, wallV) * shade : glm::vec3(0.5f);
						}
						else {
							color = doorBlendTex ? doorBlendTex->sample(hit.hitPt, wallV) * shade : (wallTex ? wallTex->sample(hit.hitPt, wallV) * shade : glm::vec3(0.5f));
						}
					}
					else {
						color = wallTex ? wallTex->sample(hit.hitPt, wallV) * shade : glm::vec3(0.5f);
					}
				}
			}

			framebuffer[idx] = (unsigned char)(glm::clamp(color.r, 0.0f, 1.0f) * 255);
			framebuffer[idx + 1] = (unsigned char)(glm::clamp(color.g, 0.0f, 1.0f) * 255);
			framebuffer[idx + 2] = (unsigned char)(glm::clamp(color.b, 0.0f, 1.0f) * 255);
		}
	}
}