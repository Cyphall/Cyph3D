#pragma once

#include <glm/glm.hpp>
#include <cstdint>

class MathHelper
{
public:
	static bool between(int64_t number, int64_t lower, int64_t upper);
	static float fovXtoY(float fovx, float aspect);
	static std::pair<glm::vec3, glm::vec3> transformBoundingBox(const glm::mat4& matrix, const glm::vec3& boundingBoxMin, const glm::vec3& boundingBoxMax);
	
	static glm::vec3 srgbToLinear(glm::vec3 color);
	static glm::vec3 linearToSrgb(glm::vec3 color);
	
};