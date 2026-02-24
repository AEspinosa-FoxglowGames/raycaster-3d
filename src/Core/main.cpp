#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include "Map.h"
#include "Player.h"
#include "Shader.h"
#include "Raycaster.h"
#include "Texture.h"
#include "Renderer.h"
#include "HUD.h"
#include <unordered_map>
#include "World.h"

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const int screenW = 800;
	const int screenH = 600;

	const int WindowW = 1600;
	const int WindowH = 1200;

	GLFWwindow* window = glfwCreateWindow(WindowW, WindowH, "RPGExp", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, WindowW, WindowH);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	Renderer renderer(screenW, screenH, "assets/shaders/screen.vert", "assets/shaders/screen.frag");

	HUD hud(renderer, screenW, screenH);

	// CPU texture for manual sampling
	CPUTexture wallTex = loadCPUTexture("assets/textures/wall.png");
	CPUTexture concreteTex = loadCPUTexture("assets/textures/GrayConcrete.jpg");
	CPUTexture StoneTileTex = loadCPUTexture("assets/textures/StoneTile.png");
	CPUTexture BlackBrickTex = loadCPUTexture("assets/textures/BlackBrick.jpg");

	

	std::unordered_map<EntityType, CPUTexture> entityTextures;
	entityTextures[EntityType::Note] = loadCPUTexture("assets/textures/Paper.jpg");

	validateEntities(entities);

	Player player;
	float lastTime = glfwGetTime();

	double xpos = 0, ypos = 0;
	double lastX = 400, lastY = 300;
	float sensitivity = 0.001f;

	

	while (!glfwWindowShouldClose(window))
	{
		float now = glfwGetTime();
		float dt = now - lastTime;
		lastTime = now;

		// Input
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		//debug hp
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
			player.health = std::max(0, player.health - 1);
			


		glfwGetCursorPos(window, &xpos, &ypos);
		double dx = xpos - lastX;
		lastX = xpos;
		lastY = ypos;
		player.angle += (float)dx * sensitivity;

		glm::vec2 dir = { cosf(player.angle), sinf(player.angle) };
		glm::vec2 right = { -dir.y, dir.x };
		glm::vec2 newPos = player.pos;

		if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS|| glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)&&player.stamina>0 && !player.exhausted)
		{
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) newPos += dir * player.sprintSpeed * dt;
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) newPos -= dir * player.sprintSpeed * dt;
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) newPos -= right * player.sprintSpeed * dt;
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) newPos += right * player.sprintSpeed * dt;
			player.stamina = glm::clamp(player.stamina - 15.0f * dt, 0.0f, (float)player.maxStamina);
			player.running = true;
		}
		else
		{
			player.running = false;
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) newPos += dir * player.speed * dt;
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) newPos -= dir * player.speed * dt;
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) newPos -= right * player.speed * dt;
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) newPos += right * player.speed * dt;
		}

		if (player.stamina <= 0) player.exhausted = true;
		if (player.stamina >= player.maxStamina*0.25f) player.exhausted = false;

		if (!player.running)
		{
			//player.stamina = std::max(player.maxStamina, player.stamina + 1);
			player.stamina = glm::clamp(player.stamina + 5.0f * dt, 0.0f, (float)player.maxStamina);
		}

		if (MAP[(int)newPos.y][(int)newPos.x] == 0)
			player.pos = newPos;


		//Rendering
		renderer.renderWorld(player, wallTex, StoneTileTex, concreteTex, entities, entityTextures);
		// Upload framebuffer to GPU and draw

		hud.drawCrosshair();

		hud.drawBars(player.health,player.maxHealth,player.stamina,player.maxStamina);

		renderer.present();

		

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}