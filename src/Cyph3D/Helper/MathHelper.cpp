#include "Cyph3D/Helper/MathHelper.h"
#include <glm/glm.hpp>

bool MathHelper::between(int64_t number, int64_t lower, int64_t upper)
{
	return (unsigned)(number-lower) <= (upper-lower);
}

float MathHelper::fovXtoY(float fovx, float aspect)
{
	return 2.0f * glm::atan(glm::tan(glm::radians(fovx) * 0.5f) / aspect);
}