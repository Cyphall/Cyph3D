#pragma once

#include <glad/glad.h>
#include <tuple>
#include <optional>
#include <array>
#include "../Enums/ImageType.h"

struct TextureProperties
{
	GLenum internalFormat;
	std::array<GLint, 4> swizzle;
};

struct PixelProperties
{
	GLenum format;
	GLenum type;
};

class TextureHelper
{
public:
	static TextureProperties getTextureProperties(ImageType type);
	static PixelProperties getPixelProperties(int channelCount, int bitPerChannel);
};
