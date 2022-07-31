#pragma once

#include <glad/glad.h>
#include <array>

struct SamplerCreateInfo
{
	GLenum minFilter = GL_NEAREST_MIPMAP_LINEAR;
	GLenum magFilter = GL_LINEAR;
	GLfloat minLod = -1000;
	GLfloat maxLod = 1000;
	GLfloat lodBias = 0;
	GLenum wrapS = GL_REPEAT;
	GLenum wrapT = GL_REPEAT;
	GLenum wrapR = GL_REPEAT;
	std::array<GLfloat, 4> borderColor = {0.0f, 0.0f, 0.0f, 0.0f};
	GLenum compareFunc = GL_NONE;
	GLboolean anisotropicFiltering = false;
};