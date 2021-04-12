#include "Image.h"
#include <stdexcept>
#include "../Helper/TextureHelper.h"
#include <fmt/core.h>

void Image::finishLoading(const ImageLoadingData& data)
{
	TextureCreateInfo createInfo;
	createInfo.size = data.data.getSize();
	createInfo.internalFormat = data.internalFormat;
	createInfo.textureFiltering = LINEAR;
	createInfo.useMipmaps = true;
	createInfo.swizzle = data.swizzle;
	
	_resource = std::make_unique<Texture>(createInfo);
	
	PixelProperties properties = TextureHelper::getPixelProperties(data.data.getChannelCount(), data.data.getBitPerChannel());
	_resource->setData(data.data.getPtr(), properties.format, properties.type);
}

ImageLoadingData Image::loadFromFile(const std::string& name, ImageType type)
{
	std::string path = fmt::format("resources/{}", name);
	
	ImageLoadingData imageData{};
	
	imageData.data = StbImage(path);
	
	if (!imageData.data.isValid())
	{
		throw std::runtime_error(fmt::format("Unable to load image {} from disk", path));
	}
	
	TextureProperties properties = TextureHelper::getTextureProperties(type);
	
	imageData.internalFormat = properties.internalFormat;
	imageData.swizzle = properties.swizzle;
	
	return imageData;
}
