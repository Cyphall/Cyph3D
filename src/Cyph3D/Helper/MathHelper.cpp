#include "MathHelper.h"

#include <glm/glm.hpp>
#include <array>

bool MathHelper::between(int64_t number, int64_t lower, int64_t upper)
{
	return (unsigned)(number-lower) <= (upper-lower);
}

float MathHelper::fovXtoY(float fovx, float aspect)
{
	return 2.0f * glm::atan(glm::tan(glm::radians(fovx) * 0.5f) / aspect);
}

std::pair<glm::vec3, glm::vec3> MathHelper::transformBoundingBox(const glm::mat4& matrix, const glm::vec3& boundingBoxMin, const glm::vec3& boundingBoxMax)
{
	std::array<glm::vec3, 8> boundingBoxVertices = {
		glm::vec3(boundingBoxMin.x, boundingBoxMin.y, boundingBoxMin.z),
		glm::vec3(boundingBoxMin.x, boundingBoxMin.y, boundingBoxMax.z),
		glm::vec3(boundingBoxMin.x, boundingBoxMax.y, boundingBoxMin.z),
		glm::vec3(boundingBoxMin.x, boundingBoxMax.y, boundingBoxMax.z),
		glm::vec3(boundingBoxMax.x, boundingBoxMin.y, boundingBoxMin.z),
		glm::vec3(boundingBoxMax.x, boundingBoxMin.y, boundingBoxMax.z),
		glm::vec3(boundingBoxMax.x, boundingBoxMax.y, boundingBoxMin.z),
		glm::vec3(boundingBoxMax.x, boundingBoxMax.y, boundingBoxMax.z)
	};
	
	glm::vec3 min(std::numeric_limits<float>::max());
	glm::vec3 max(std::numeric_limits<float>::lowest());
	
	for (const glm::vec3& vertex : boundingBoxVertices)
	{
		glm::vec3 transformedVertex = glm::vec3(matrix * glm::vec4(vertex, 1));
		
		min = glm::min(min, transformedVertex);
		max = glm::max(max, transformedVertex);
	}
	
	return {min, max};
}

glm::vec3 MathHelper::srgbToLinear(glm::vec3 color)
{
	glm::bvec3 cutoff = lessThan(color, glm::vec3(0.04045f));
	glm::vec3 higher = pow((color + glm::vec3(0.055f))/glm::vec3(1.055f), glm::vec3(2.4f));
	glm::vec3 lower = color/glm::vec3(12.92f);
	
	return glm::mix(higher, lower, cutoff);
}

glm::vec3 MathHelper::linearToSrgb(glm::vec3 color)
{
	glm::bvec3 cutoff = lessThan(color, glm::vec3(0.0031308f));
	glm::vec3 higher = glm::vec3(1.055f)*pow(color, glm::vec3(1.0f/2.4f)) - glm::vec3(0.055f);
	glm::vec3 lower = color * glm::vec3(12.92f);
	
	return glm::mix(higher, lower, cutoff);
}