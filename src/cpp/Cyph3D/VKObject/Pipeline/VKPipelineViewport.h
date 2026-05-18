#pragma once

#include <glm/glm.hpp>

namespace c3d
{
struct VKPipelineViewport
{
	glm::vec2 offset = {};
	glm::vec2 size = {};
	glm::vec2 depthRange = {};
};
}