#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <array>

struct CubemapCreateInfo
{
	glm::ivec2 size;
	GLenum internalFormat = GL_RGB8;
	GLenum textureFiltering = GL_NEAREST;
	std::array<GLint, 4> swizzle = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
};