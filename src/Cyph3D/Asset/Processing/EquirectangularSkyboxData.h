#pragma once

#include <array>
#include <cstddef>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

struct EquirectangularSkyboxData
{
	vk::Format format;
	glm::uvec2 size;
	std::array<std::vector<std::vector<std::byte>>, 6> faces; // Face<Level<Data>>
};