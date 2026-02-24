#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

struct CPUTexture {
	std::vector<unsigned char> pixels;
	int width, height, channels;
	glm::vec3 sample(float u, float v) const;
};

CPUTexture loadCPUTexture(const char* path);