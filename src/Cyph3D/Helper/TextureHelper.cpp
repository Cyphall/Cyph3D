#include "TextureHelper.h"

#include <format>
#include <stdexcept>

TextureProperties TextureHelper::getTextureProperties(ImageType type)
{
	switch (type)
	{
		case COLOR_SRGB:
			return TextureProperties
			{
					.internalFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
					.swizzle = {GL_RED, GL_GREEN, GL_BLUE, GL_ONE}
			};
		case NORMAL_MAP:
			return TextureProperties
			{
					.internalFormat = GL_COMPRESSED_RG_RGTC2,
					.swizzle = {GL_RED, GL_GREEN, GL_ZERO, GL_ZERO}
			};
		case GRAYSCALE:
			return TextureProperties
			{
					.internalFormat = GL_COMPRESSED_RED_RGTC1,
					.swizzle = {GL_RED, GL_RED, GL_RED, GL_ONE}
			};
		default:
			throw std::runtime_error(std::format("Unknown ImageType value: {}", static_cast<int>(type)));
	}
}

PixelProperties TextureHelper::getPixelProperties(int channelCount, int bitPerChannel)
{
	PixelProperties properties{};
	
	switch (channelCount)
	{
		case 1:
			properties.format = GL_RED;
			break;
		case 2:
			properties.format = GL_RG;
			break;
		case 3:
			properties.format = GL_RGB;
			break;
		case 4:
			properties.format = GL_RGBA;
			break;
		default:
			throw std::runtime_error("This exception is not supposed to be reachable");
	}
	
	switch (bitPerChannel)
	{
		case 8:
			properties.type = GL_UNSIGNED_BYTE;
			break;
		case 16:
			properties.type = GL_UNSIGNED_SHORT;
			break;
		case 32:
			properties.type = GL_FLOAT;
			break;
		default:
			throw std::runtime_error("This exception is not supposed to be reachable");
	}
	
	return properties;
}