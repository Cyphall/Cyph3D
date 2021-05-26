#include "Skybox.h"
#include "../Helper/JsonHelper.h"
#include "../Helper/TextureHelper.h"
#include <format>

void Skybox::finishLoading(const SkyboxLoadingData& data)
{
	CubemapCreateInfo createInfo;
	createInfo.size = data.data[0].getSize();
	createInfo.internalFormat = data.internalFormat;
	createInfo.textureFiltering = GL_LINEAR;
	createInfo.swizzle = data.swizzle;
	
	_resource = std::make_unique<Cubemap>(createInfo);
	
	for (int i = 0; i < data.data.size(); ++i)
	{
		PixelProperties properties = TextureHelper::getPixelProperties(data.data[i].getChannelCount(), data.data[i].getBitPerChannel());
		
		_resource->setData(data.data[i].getPtr(), i, properties.format, properties.type);
	}
}

SkyboxLoadingData Skybox::loadFromFile(const std::string& name)
{
	std::string path = std::format("resources/skyboxes/{}", name);
	
	nlohmann::ordered_json root = JsonHelper::loadJsonFromFile(std::format("{}/skybox.json", path));
	
	std::string facePaths[6] =
	{
		std::format("{}/{}", path, root["right"].get<std::string>()),
		std::format("{}/{}", path, root["left"].get<std::string>()),
		std::format("{}/{}", path, root["down"].get<std::string>()),
		std::format("{}/{}", path, root["up"].get<std::string>()),
		std::format("{}/{}", path, root["front"].get<std::string>()),
		std::format("{}/{}", path, root["back"].get<std::string>())
	};
	
	SkyboxLoadingData skyboxData{};
	
	int skyboxComp;
	glm::ivec2 skyboxSize;
	
	bool firstIteration = true;
	for (int i = 0; i < 6; ++i)
	{
		skyboxData.data[i] = StbImage(facePaths[i].c_str());
		
		if (!skyboxData.data[i].isValid())
		{
			throw std::runtime_error(std::format("Unable to load image {} from disk", path));
		}
		
		if (firstIteration)
		{
			skyboxComp = skyboxData.data[i].getChannelCount();
			skyboxSize = skyboxData.data[i].getSize();
			firstIteration = false;
		}
		
		if (skyboxData.data[i].getChannelCount() != skyboxComp || skyboxData.data[i].getSize() != skyboxSize)
		{
			throw std::runtime_error(std::format("Skybox {} have images with different formats", name));
		}
	}
	
	TextureProperties properties = TextureHelper::getTextureProperties(COLOR_SRGB);
	
	skyboxData.internalFormat = properties.internalFormat;
	skyboxData.swizzle = properties.swizzle;
	
	return skyboxData;
}

float Skybox::getRotation() const
{
	return _rotation;
}

void Skybox::setRotation(float rotation)
{
	_rotation = rotation;
}
