#pragma once

#include <glm/glm.hpp>

struct VKPipelineScissor
{
	glm::ivec2 offset = {};
	glm::uvec2 size = {};
};