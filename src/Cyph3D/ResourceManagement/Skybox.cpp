#include "Skybox.h"

#include "Cyph3D/GLObject/CreateInfo/CubemapCreateInfo.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/ResourceManagement/StbImage.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/GLObject/GLFence.h"

#include <chrono>
#include <format>
#include <array>

struct Skybox::LoadData
{
	ResourceManager* rm;

	// step 1: read the image and fill the properties
	std::array<StbImage, 6> imageData;
	CubemapCreateInfo cubemapCreateInfo;
	std::array<PixelProperties, 6> pixelProperties;

	// step 2: create GLTexture and fill it
	std::unique_ptr<GLFence> fence;

	// step 3: wait for the fence
};

Skybox::Skybox(const std::string& name, ResourceManager& rm):
	Resource(name)
{
	_loadData = std::make_unique<LoadData>();
	_loadData->rm = &rm;

	Logger::info(std::format("Loading skybox \"{}\"", getName()));
	_loadData->rm->addThreadPoolTask(std::bind(&Skybox::load_step1_tp, this));
}

Skybox::~Skybox()
{}

float Skybox::getRotation() const
{
	return _rotation;
}

void Skybox::setRotation(float rotation)
{
	_rotation = rotation;
}

void Skybox::load_step1_tp()
{
	std::string path = std::format("resources/skyboxes/{}", _name);

	nlohmann::ordered_json root = JsonHelper::loadJsonFromFile(std::format("{}/skybox.json", path));

	std::string facePaths[6] = {
		std::format("{}/{}", path, root["right"].get<std::string>()),
		std::format("{}/{}", path, root["left"].get<std::string>()),
		std::format("{}/{}", path, root["down"].get<std::string>()),
		std::format("{}/{}", path, root["up"].get<std::string>()),
		std::format("{}/{}", path, root["front"].get<std::string>()),
		std::format("{}/{}", path, root["back"].get<std::string>())
	};

	int skyboxComp;
	glm::ivec2 skyboxSize;

	bool firstIteration = true;
	for (int i = 0; i < 6; ++i)
	{
		_loadData->imageData[i] = StbImage(facePaths[i].c_str());

		if (!_loadData->imageData[i].isValid())
		{
			throw std::runtime_error(std::format("Unable to load image {} from disk", path));
		}

		if (firstIteration)
		{
			skyboxComp = _loadData->imageData[i].getChannelCount();
			skyboxSize = _loadData->imageData[i].getSize();
			firstIteration = false;
		}

		if (_loadData->imageData[i].getChannelCount() != skyboxComp || _loadData->imageData[i].getSize() != skyboxSize)
		{
			throw std::runtime_error(std::format("Skybox {} have images with different formats", _name));
		}

		_loadData->pixelProperties[i] = TextureHelper::getPixelProperties(_loadData->imageData[i].getChannelCount(), _loadData->imageData[i].getBitPerChannel());
	}

	TextureProperties textureProperties = TextureHelper::getTextureProperties(COLOR_SRGB);

	_loadData->cubemapCreateInfo.size = _loadData->imageData[0].getSize();
	_loadData->cubemapCreateInfo.internalFormat = textureProperties.internalFormat;
	_loadData->cubemapCreateInfo.textureFiltering = GL_LINEAR;
	_loadData->cubemapCreateInfo.swizzle = textureProperties.swizzle;

	_loadData->rm->addMainThreadTask(std::bind(&Skybox::load_step2_mt, this));
}

bool Skybox::load_step2_mt()
{
	_resource = std::make_unique<GLCubemap>(_loadData->cubemapCreateInfo);
	
	for (int i = 0; i < _loadData->imageData.size(); i++)
	{
		_resource->setData(_loadData->imageData[i].getPtr(), i, _loadData->pixelProperties[i].format, _loadData->pixelProperties[i].type);
	}

	_loadData->fence = std::make_unique<GLFence>();

	_loadData->rm->addMainThreadTask(std::bind(&Skybox::load_step3_mt, this));
	return true;
}

bool Skybox::load_step3_mt()
{
	if (!_loadData->fence->isSignaled())
	{
		return false;
	}

	_loadData.reset();
	_ready = true;
	Logger::info(std::format("Skybox \"{}\" loaded", getName()));
	return true;
}