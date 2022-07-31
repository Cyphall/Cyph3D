#pragma once

#include "Cyph3D/Enums/ImageType.h"

#include <glad/glad.h>
#include <array>
#include <optional>
#include <tuple>

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