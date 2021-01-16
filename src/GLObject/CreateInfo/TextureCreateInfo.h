#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <optional>
#include <array>

enum TextureFiltering
{
	NEAREST,
	LINEAR
};

struct TextureCreateInfo
{
	glm::ivec2 size;
	GLenum internalFormat = GL_RGB8;
	TextureFiltering textureFiltering = NEAREST;
	bool useMipmaps = false;
	bool isShadowMap = false;
	std::array<GLint, 4> swizzle = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
};