#pragma once

#include <glm/glm.hpp>

namespace c3d
{
struct PositionVertexData
{
	glm::vec3 position;
};

struct MaterialVertexData
{
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec4 tangent;
};
}
