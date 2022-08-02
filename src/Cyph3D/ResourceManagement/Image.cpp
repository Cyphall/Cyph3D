#include "Image.h"

#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/ResourceManagement/StbImage.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"

#include <chrono>
#include <format>
#include <stdexcept>

struct Image::LoadData
{
	ResourceManager* rm;

	// step 1: read the image and fill the properties
	ImageType type;
	StbImage imageData;
	TextureCreateInfo textureCreateInfo;
	PixelProperties pixelProperties;
	
	// step 2: create GLTexture and fill it
	GLsync fence;
	
	// step 3: wait for the fence
};

Image::Image(const std::string& name, ImageType type, ResourceManager& rm):
	Resource(name)
{
	_loadData = std::make_unique<LoadData>();
	_loadData->rm = &rm;
	_loadData->type = type;

	Logger::info(std::format("Loading image \"{}\"", getName()));
	_loadData->rm->addThreadPoolTask(std::bind(&Image::load_step1_tp, this));
}

Image::~Image()
{}

void Image::load_step1_tp()
{
	std::string path = std::format("resources/{}", _name);

	_loadData->imageData = StbImage(path);

	if (!_loadData->imageData.isValid())
	{
		throw std::runtime_error(std::format("Unable to load image {} from disk", path));
	}

	TextureProperties textureProperties = TextureHelper::getTextureProperties(_loadData->type);

	_loadData->textureCreateInfo.size = _loadData->imageData.getSize();
	_loadData->textureCreateInfo.internalFormat = textureProperties.internalFormat;
	_loadData->textureCreateInfo.minFilter = GL_LINEAR_MIPMAP_LINEAR;
	_loadData->textureCreateInfo.magFilter = GL_LINEAR;
	_loadData->textureCreateInfo.anisotropicFiltering = true;
	_loadData->textureCreateInfo.swizzle = textureProperties.swizzle;

	_loadData->pixelProperties = TextureHelper::getPixelProperties(_loadData->imageData.getChannelCount(), _loadData->imageData.getBitPerChannel());
	
	_loadData->rm->addMainThreadTask(std::bind(&Image::load_step2_mt, this));
}

bool Image::load_step2_mt()
{
	_resource = std::make_unique<GLTexture>(_loadData->textureCreateInfo);
	_resource->setData(_loadData->imageData.getPtr(), _loadData->pixelProperties.format, _loadData->pixelProperties.type);

	_loadData->fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	_loadData->rm->addMainThreadTask(std::bind(&Image::load_step3_mt, this));
	return true;
}

bool Image::load_step3_mt()
{
	if (glClientWaitSync(_loadData->fence, 0, 0) != GL_ALREADY_SIGNALED)
	{
		return false;
	}

	_loadData.reset();
	_ready = true;
	Logger::info(std::format("Image \"{}\" loaded", getName()));
	return true;
}