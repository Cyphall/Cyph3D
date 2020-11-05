#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <optional>
#include <array>

struct CubemapCreateInfo
{
	glm::ivec2 size;
	GLenum internalFormat = GL_RGB8;
	GLenum textureFiltering = GL_NEAREST;
	std::array<GLint, 4> swizzle;
};