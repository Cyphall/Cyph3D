#pragma once

#include <array>

struct SamplerCreateInfo
{
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
	bool enableAnisotropicFiltering = false;
};
