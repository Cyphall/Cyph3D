#pragma once

#include <glm/glm.hpp>

class ILoggerColor
{
public:
	virtual ~ILoggerColor() = default;

	virtual void setColor(glm::u8vec3 color) = 0;
	virtual void resetColor() = 0;
};