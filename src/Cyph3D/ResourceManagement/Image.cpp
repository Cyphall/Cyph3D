#include "Image.h"
#include <stdexcept>
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/ResourceManagement/StbImage.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include <format>
#include <chrono>

void Image::loadResourceImpl(ImageType type)
{
	std::string path = std::format("resources/{}", _name);
	
	StbImage data = StbImage(path);
	
	if (!data.isValid())
	{
		throw std::runtime_error(std::format("Unable to load image {} from disk", path));
	}
	
	TextureProperties textureProperties = TextureHelper::getTextureProperties(type);
	
	TextureCreateInfo createInfo;
	createInfo.size = data.getSize();
	createInfo.internalFormat = textureProperties.internalFormat;
	createInfo.minFilter = GL_LINEAR_MIPMAP_LINEAR;
	createInfo.magFilter = GL_LINEAR;
	createInfo.anisotropicFiltering = true;
	createInfo.swizzle = textureProperties.swizzle;
	
	_resource = std::make_unique<Texture>(createInfo);
	
	PixelProperties pixelProperties = TextureHelper::getPixelProperties(data.getChannelCount(), data.getBitPerChannel());
	_resource->setData(data.getPtr(), pixelProperties.format, pixelProperties.type);
	
	GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	auto timeout = std::chrono::seconds(10);
	glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
}