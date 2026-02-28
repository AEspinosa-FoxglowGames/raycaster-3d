#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "Map.h"
#include "Player.h"
#include "Shader.h"
#include "Raycaster.h"
#include "Texture.h"
#include "Renderer.h"
#include "HUD.h"
#include "World.h"

int main()
{
	// -----------------------------------------------------------------------
	// Window / GL setup
	// -----------------------------------------------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	constexpr int SCREEN_W = 800;
	constexpr int SCREEN_H = 600;
	constexpr int WINDOW_W = 1600;
	constexpr int WINDOW_H = 1200;

	GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Foxglow Horror", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, WINDOW_W, WINDOW_H);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// -----------------------------------------------------------------------
	// Renderer + HUD
	// -----------------------------------------------------------------------
	Renderer renderer(SCREEN_W, SCREEN_H,
		"assets/shaders/screen.vert",
		"assets/shaders/screen.frag");
	HUD hud(renderer, SCREEN_W, SCREEN_H);

	// -----------------------------------------------------------------------
	// Texture registry
	// -----------------------------------------------------------------------
	TextureRegistry textures;
	textures.setWall(TileType::WallStone, loadCPUTexture("assets/textures/walls/wall.png"));
	textures.setWall(TileType::WallBrick, loadCPUTexture("assets/textures/walls/BlackBrick.jpg"));
	textures.setWall(TileType::WallConcrete, loadCPUTexture("assets/textures/walls/GrayConcrete.jpg"));
	textures.setWall(TileType::WallSealed, loadCPUTexture("assets/textures/walls/StoneTile.png"));
	textures.setWall(TileType::DoorClosedH, loadCPUTexture("assets/textures/doors/creepydoor.jpg"));
	textures.setWall(TileType::DoorClosedV, loadCPUTexture("assets/textures/doors/creepydoor.jpg"));
	textures.setWall(TileType::DoorLocked, loadCPUTexture("assets/textures/walls/wall.png")); // swap for locked door texture
	textures.setWall(TileType::DoorFrame, loadCPUTexture("assets/textures/walls/GrayConcrete.jpg")); // frame sides match surrounding wall
	textures.floorTex = loadCPUTexture("assets/textures/walls/StoneTile.png");
	textures.ceilingTex = loadCPUTexture("assets/textures/walls/GrayConcrete.jpg");
	textures.setEntity(EntityType::Note, loadCPUTexture("assets/textures/items/Paper.jpg"));

	// -----------------------------------------------------------------------
	// World validation
	// -----------------------------------------------------------------------
	validateEntities(entities);

	// -----------------------------------------------------------------------
	// Player + input state
	// -----------------------------------------------------------------------
	Player player;
	float lastTime = (float)glfwGetTime();
	double lastX = WINDOW_W / 2.0;
	double lastY = WINDOW_H / 2.0;
	constexpr float MOUSE_SENSITIVITY = 0.001f;

	// -----------------------------------------------------------------------
	// Main loop
	// -----------------------------------------------------------------------
	while (!glfwWindowShouldClose(window))
	{
		float now = (float)glfwGetTime();
		float dt = now - lastTime;
		lastTime = now;

		// --- Input ---
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// Debug: drain HP with G
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
			player.health = std::max(0, player.health - 1);

		// Mouse look
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		double dx = xpos - lastX;
		lastX = xpos;
		lastY = ypos;
		player.angle += (float)dx * MOUSE_SENSITIVITY;

		// Movement direction vectors
		glm::vec2 forward = { cosf(player.angle), sinf(player.angle) };
		glm::vec2 right = { -forward.y, forward.x };
		glm::vec2 newPos = player.pos;

		// BUG FIX: was checking LEFT_SHIFT twice; now checks left OR right shift
		bool sprinting = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
			&& player.stamina > 0.0f
			&& !player.exhausted;

		float moveSpeed = sprinting ? player.sprintSpeed : player.speed;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) newPos += forward * moveSpeed * dt;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) newPos -= forward * moveSpeed * dt;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) newPos -= right * moveSpeed * dt;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) newPos += right * moveSpeed * dt;

		player.running = sprinting;

		// Stamina drain/regen
		if (sprinting) {
			player.stamina = glm::clamp(player.stamina - 15.0f * dt, 0.0f, (float)player.maxStamina);
		}
		else {
			player.stamina = glm::clamp(player.stamina + 5.0f * dt, 0.0f, (float)player.maxStamina);
		}

		// Exhaustion: triggered at 0, clears at 25%
		if (player.stamina <= 0.0f)                              player.exhausted = true;
		if (player.stamina >= player.maxStamina * 0.25f)         player.exhausted = false;

		// Collision: separate X and Y so player can slide along walls
		glm::vec2 testX = { newPos.x, player.pos.y };
		glm::vec2 testY = { player.pos.x, newPos.y };
		if (!isSolid(getTile((int)testX.x, (int)testX.y))) player.pos.x = testX.x;
		if (!isSolid(getTile((int)testY.x, (int)testY.y))) player.pos.y = testY.y;

		// --- Render ---
		renderer.renderWorld(player, entities, textures);
		hud.drawCrosshair();
		hud.drawBars(player.health, player.maxHealth, player.stamina, player.maxStamina);
		renderer.present();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}