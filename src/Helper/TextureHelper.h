#pragma once

#include <glad/glad.h>
#include <tuple>
#include <optional>
#include <array>

struct TextureInfo
{
	GLenum internalFormat;
	GLenum pixelFormat;
	std::optional<std::array<GLint, 4>> swizzle;
};

TextureInfo getTextureInfo(int componentCount, bool compressed, bool sRGB);


