#include "Image.h"
#include <stdexcept>
#include "../Helper/TextureHelper.h"
#include <fmt/core.h>

void Image::finishLoading(const ImageLoadingData& data)
{
	TextureCreateInfo createInfo;
	createInfo.size = data.data.getSize();
	createInfo.internalFormat = data.internalFormat;
	createInfo.textureFiltering = GL_LINEAR;
	createInfo.useMipmaps = true;
	createInfo.swizzle = data.swizzle;
	
	_resource = std::make_unique<Texture>(createInfo);
	_resource->setData(data.data.getPtr(), data.pixelFormat, data.data.getBitPerChannel() == 16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE);
}

ImageLoadingData Image::loadFromFile(const std::string& name, bool sRGB, bool compressed)
{
	std::string path = fmt::format("resources/materials/{}", name);
	
	ImageLoadingData imageData{};
	
	imageData.data = StbImage(path);
	
	if (!imageData.data.isValid())
	{
		throw std::runtime_error(fmt::format("Unable to load image {} from disk", path));
	}
	
	TextureInfo info = TextureHelper::getTextureInfo(imageData.data.getChannels(), compressed, sRGB);
	
	imageData.internalFormat = info.internalFormat;
	imageData.pixelFormat = info.pixelFormat;
	imageData.swizzle = info.swizzle;
	
	return imageData;
}
