#include "Image.h"
#include <stdexcept>
#include <iostream>
#include "../Helper/TextureHelper.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <fmt/core.h>

void Image::finishLoading(const ImageLoadingData& data)
{
	TextureCreateInfo createInfo;
	createInfo.size = data.size;
	createInfo.internalFormat = data.internalFormat;
	createInfo.textureFiltering = GL_LINEAR;
	createInfo.useMipmaps = true;
	createInfo.swizzle = data.swizzle;
	
	_resource = std::make_unique<Texture>(createInfo);
	_resource->setData(data.data.get(), data.pixelFormat);
}

ImageLoadingData Image::loadFromFile(const std::string& name, bool sRGB, bool compressed)
{
	std::string path = fmt::format("resources/materials/{}", name);
	
	ImageLoadingData imageData{};
	
	int comp;
	
	imageData.data.reset(stbi_load(path.c_str(), &imageData.size.x, &imageData.size.y, &comp, 0));
	
	if (imageData.data == nullptr)
	{
		throw std::runtime_error(fmt::format("Unable to load image {} from disk", path));
	}
	
	TextureInfo info = getTextureInfo(comp, compressed, sRGB);
	
	imageData.internalFormat = info.internalFormat;
	imageData.pixelFormat = info.pixelFormat;
	imageData.swizzle = info.swizzle;
	
	return imageData;
}
