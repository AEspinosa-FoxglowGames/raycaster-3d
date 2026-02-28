
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"

glm::vec3 CPUTexture::sample(float u, float v) const
{
	int x = (int)(u * width) % width;
	int y = (int)(v * height) % height;
	if (x < 0) x += width;
	if (y < 0) y += height;
	int idx = (y * width + x) * channels;
	return { pixels[idx] / 255.0f, pixels[idx + 1] / 255.0f, pixels[idx + 2] / 255.0f };
}

CPUTexture loadCPUTexture(const char* path)
{
	CPUTexture tex;
	unsigned char* data = stbi_load(path, &tex.width, &tex.height, &tex.channels, 0);
	if (!data) {
		std::cerr << "Failed to load texture: " << path << "\n";
		return tex;
	}
	tex.pixels.assign(data, data + tex.width * tex.height * tex.channels);
	stbi_image_free(data);
	return tex;
}