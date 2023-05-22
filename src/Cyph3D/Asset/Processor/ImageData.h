#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <cstddef>

enum class ImageType
{
	ColorSrgb,
	NormalMap,
	Grayscale,
	Skybox
};

struct ImageLevel
{
	std::vector<std::byte> data;
};

struct ImageData
{
	vk::Format format;
	glm::uvec2 size;
	std::vector<ImageLevel> levels;
};