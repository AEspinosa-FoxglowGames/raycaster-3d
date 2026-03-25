#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <iostream>

struct CPUTexture {
	std::vector<unsigned char> pixels;
	int width = 0;
	int height = 0;
	int channels = 0;

	bool empty() const { return pixels.empty(); }
	bool hasAlpha() const { return channels == 4; }

	// Sample RGB (0-1 range). Wraps on both axes.
	glm::vec3 sample(float u, float v) const;

	// Sample alpha channel (0-1). Returns 1.0 (fully opaque) if no alpha channel.
	// For sprites with black backgrounds (channels < 4), use luminanceAlpha() instead.
	float sampleAlpha(float u, float v) const;

	// Returns 0 if pixel is near-black (black-bg sprite transparency fallback).
	// Returns 1 otherwise. Use when PNG has no alpha channel.
	float luminanceAlpha(float u, float v, float threshold = 0.08f) const;
};

CPUTexture loadCPUTexture(const char* path);