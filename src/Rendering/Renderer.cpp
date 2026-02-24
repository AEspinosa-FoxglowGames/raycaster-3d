#include "Renderer.h"
#include "Shader.h"
#include "Raycaster.h"
#include "Texture.h"

Renderer::Renderer(int w, int h, const std::string& shaderVert, const std::string& shaderFrag)
{

	screenW = w;
	screenH = h;


	// Shaders
	screenShader = makeShader(shaderVert.c_str(), shaderFrag.c_str()); // assigns to class member
	// CPU framebuffer
	framebuffer.resize(screenW * screenH * 3, 0);                      // resizes class member

	

	// GPU texture for framebuffer
	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenW, screenH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Fullscreen quad
	float quadVerts[] = {
		-1,-1, 0,1,
		 1,-1, 1,1,
		 1, 1, 1,0,
		-1,-1, 0,1,
		 1, 1, 1,0,
		-1, 1, 0,0,
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

void Renderer::renderWorld(const Player& player, const CPUTexture& wallTex, const CPUTexture& floorTex, const CPUTexture& ceilingTex, const std::vector<Entity>& entities,const std::unordered_map<EntityType, CPUTexture>& entityTextures)
{
	float fov = glm::radians(66.0f);
	float halfTan = tanf(fov / 2.0f);
	
	for (int col = 0; col < screenW; col++)
	{
		float camX = 2.0f * col / screenW - 1.0f;
		float rayAngle = player.angle + atanf(camX * halfTan);

		RayHit hit = castRay(player.pos, rayAngle);
		glm::vec2 rayDir = { cosf(rayAngle), sinf(rayAngle) };

		

		// How many pixels tall is this wall strip
		int wallPx = (int)(screenH / hit.dist);
		int wallTop = screenH / 2 - wallPx / 2;
		int wallBottom = screenH / 2 + wallPx / 2;

		float shade = glm::clamp(1.0f / (hit.dist * 1.5f), 0.0f, 1.0f);

		// Check for wall decal on this column's wall
		const Entity* decal = nullptr;
		for (const auto& e : entities) {
			if ((int)e.pos.x == hit.wallX && (int)e.pos.y == hit.wallY) {
				decal = &e;
				break;
			}
		}

		if (hit.hitNS) shade *= 0.6f;

		for (int row = 0; row < screenH; row++)
		{
			int idx = (row * screenW + col) * 3;
			glm::vec3 color;

			if (row < wallTop) {
				// Ceiling
				float rowDist = (screenH / 2.0f) / (screenH / 2.0f - row);
				
				glm::vec2 CeilingPos = player.pos + rowDist * rayDir;
				float u = fmodf(CeilingPos.x, 1.0f);
				float v = fmodf(CeilingPos.y, 1.0f);
				if (u < 0) u += 1.0f;
				if (v < 0) v += 1.0f;
				float ceilingShade = glm::clamp(1.0f / (rowDist * 1.5f), 0.0f, 1.0f);
				color = ceilingTex.sample(u, v) * ceilingShade*0.2f;//Darkening
			}
			else if (row >= wallBottom) {
				// Floor
				float rowDist = (screenH / 2.0f) / (row - screenH / 2.0f);
				glm::vec2 floorPos = player.pos + rowDist * rayDir;
				float u = fmodf(floorPos.x, 1.0f);
				float v = fmodf(floorPos.y, 1.0f);
				if (u < 0) u += 1.0f;
				if (v < 0) v += 1.0f;
				float floorShade = glm::clamp(1.0f / (rowDist * 1.5f), 0.0f, 1.0f);
				color = floorTex.sample(u, v) * floorShade;
			}
			else {
				float u = hit.hitPt;
				float v = (float)(row - wallTop) / (float)(wallPx);

				float decalMin = 0.45f;
				float decalMax = 0.65f;

				if (decal && hit.hitPt >= decalMin && hit.hitPt <= decalMax
					&& v >= decalMin && v <= decalMax)
				{ 
					float decalU = (hit.hitPt - decalMin) / (decalMax - decalMin);
					float decalV = (v - decalMin) / (decalMax - decalMin);
					glm::vec3 decalColor = entityTextures.at(decal->type).sample(decalU, decalV);
					if (decalColor.r + decalColor.g + decalColor.b > 0.1f)
						color = decalColor * shade;
					else
						color = wallTex.sample(u, v) * shade;
				}
				else {
					color = wallTex.sample(u, v) * shade;
				}
			}

			framebuffer[idx] = (unsigned char)(glm::clamp(color.r, 0.0f, 1.0f) * 255);
			framebuffer[idx + 1] = (unsigned char)(glm::clamp(color.g, 0.0f, 1.0f) * 255);
			framebuffer[idx + 2] = (unsigned char)(glm::clamp(color.b, 0.0f, 1.0f) * 255);
		}
	}
	
}

void Renderer::drawRect(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b)
{
	for (int col = y; col < y+h; col++)
	{
		for (int row = x; row < x+w; row++)
		{
			setPixel(row, col, r,g,b);
		}
	}
}