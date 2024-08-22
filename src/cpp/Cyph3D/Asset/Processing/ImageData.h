#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

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