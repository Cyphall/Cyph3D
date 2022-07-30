#pragma once

#include <cstdint>

class MathHelper
{
public:
	static bool between(int64_t number, int64_t lower, int64_t upper);
	static float fovXtoY(float fovx, float aspect);
};