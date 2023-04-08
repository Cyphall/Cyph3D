#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <cstddef>

enum class ImageType
{
	ColorSrgb,
	NormalMap,
	Grayscale
};

struct ImageData
{
	vk::Format format;
	glm::uvec2 size;
	std::vector<std::byte> data;
};