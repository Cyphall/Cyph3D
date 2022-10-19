#pragma once

#include "Cyph3D/Enums/TextureType.h"

#include <glad/glad.h>
#include <array>

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
	static TextureProperties getTextureProperties(TextureType type);
	static PixelProperties getPixelProperties(int channelCount, int bitPerChannel);
};