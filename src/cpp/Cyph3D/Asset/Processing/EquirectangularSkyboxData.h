#pragma once

#include <cstddef>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace c3d
{
struct EquirectangularSkyboxData
{
	vk::Format format;
	glm::uvec2 extent;
	std::vector<std::vector<std::byte>> levels; // Level<Data>
};
}
