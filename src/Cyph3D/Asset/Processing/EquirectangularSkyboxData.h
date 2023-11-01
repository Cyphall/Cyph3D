#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <cstddef>
#include <vector>
#include <array>

struct EquirectangularSkyboxData
{
	vk::Format format;
	glm::uvec2 size;
	std::array<std::vector<std::vector<std::byte>>, 6> faces; // Face<Level<Data>>
};