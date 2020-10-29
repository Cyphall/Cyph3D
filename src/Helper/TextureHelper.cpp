#include "TextureHelper.h"

TextureInfo getTextureInfo(int componentCount, bool compressed, bool sRGB)
{
	TextureInfo info;
	
	switch (componentCount)
	{
		case 1:
			info.pixelFormat = GL_RED;
			info.swizzle = {GL_RED, GL_RED, GL_RED, GL_ONE};
			if (compressed)
				info.internalFormat = sRGB ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT: GL_COMPRESSED_RED_RGTC1;
			else
				info.internalFormat = sRGB ? GL_SRGB8 : GL_R8;
			break;
		case 2:
			info.pixelFormat = GL_RG;
			info.swizzle = {GL_RED, GL_RED, GL_RED, GL_GREEN};
			if (compressed)
				info.internalFormat = sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RG_RGTC2;
			else
				info.internalFormat = sRGB ? GL_SRGB8_ALPHA8 : GL_RG8;
			break;
		case 3:
			info.pixelFormat = GL_RGB;
			if (compressed)
				info.internalFormat = sRGB ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			else
				info.internalFormat = sRGB ? GL_SRGB8 : GL_RGB8;
			break;
		case 4:
			info.pixelFormat = GL_RGBA;
			if (compressed)
				info.internalFormat = sRGB ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			else
				info.internalFormat = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
			break;
	}
	
	return info;
}
