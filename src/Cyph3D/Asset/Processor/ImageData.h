#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

struct ImageLevel
{
	std::vector<uint8_t> data;
	glm::ivec2 size;
	GLenum format;
};

struct ImageData
{
	std::vector<ImageLevel> levels;
};