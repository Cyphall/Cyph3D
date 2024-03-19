#pragma once

#include <glm/glm.hpp>

struct PositionVertexData
{
	glm::vec3 position;
};

struct FullVertexData
{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec4 tangent;
};