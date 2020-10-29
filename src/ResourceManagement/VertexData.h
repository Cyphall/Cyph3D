#pragma once

#include <glm/glm.hpp>

struct VertexData
{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
	
	VertexData(const glm::vec3& position, const glm::vec2& uv, const glm::vec3& normal, const glm::vec3& tangent):
			position(position), uv(uv), normal(normal), tangent(tangent)
	{}
};