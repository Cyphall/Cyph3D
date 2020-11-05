#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <optional>
#include <array>

struct TextureCreateInfo
{
	glm::ivec2 size;
	GLenum internalFormat = GL_RGB8;
	GLenum textureFiltering = GL_NEAREST;
	bool useMipmaps = false;
	bool isShadowMap = false;
	std::array<GLint, 4> swizzle = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
};