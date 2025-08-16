#pragma once

#include <glm/glm.hpp>

struct PositionVertexData
{
	glm::vec3 position;
};

struct MaterialVertexData
{
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec4 tangent;
};