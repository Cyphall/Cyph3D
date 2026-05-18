#pragma once

#include <glm/glm.hpp>

namespace c3d
{
struct VKPipelineScissor
{
	glm::ivec2 offset = {};
	glm::uvec2 size = {};
};
}