#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <cstddef>
#include <vector>

enum class ImageType
{
	ColorSrgb,
	NormalMap,
	Grayscale,
	Skybox
};

struct ImageData
{
	vk::Format format;
	glm::uvec2 size;
	std::vector<std::vector<std::byte>> levels; // Level<Data>
};