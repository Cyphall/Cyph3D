#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <array>

struct TextureCreateInfo
{
	glm::ivec2 size;
	GLenum internalFormat = GL_RGB8;
	GLenum minFilter = GL_NEAREST_MIPMAP_LINEAR;
	GLenum magFilter = GL_LINEAR;
	float minLod = -1000;
	float maxLod = 1000;
	float lodBias = 0;
	GLenum wrapS = GL_REPEAT;
	GLenum wrapT = GL_REPEAT;
	GLenum wrapR = GL_REPEAT;
	std::array<float, 4> borderColor = {0.0f, 0.0f, 0.0f, 0.0f};
	GLenum compareFunc = GL_NONE;
	bool anisotropicFiltering = false;
	std::array<GLint, 4> swizzle = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
	int baseLevel = 0;
	int levels = 1; // ignored if anisotropicFiltering == true
	int maxLevel = 1000;
	GLenum depthStencilTextureMode = GL_DEPTH_COMPONENT;
};