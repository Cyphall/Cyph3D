#include "Skybox.h"
#include "../Helper/JsonHelper.h"
#include "../Helper/TextureHelper.h"
#include <stb_image.h>
#include <fmt/core.h>

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
		_resource->setData(data.data[i].getPtr(), i, data.pixelFormat, data.data[i].getBitPerChannel() == 16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE);
	}
}

SkyboxLoadingData Skybox::loadFromFile(const std::string& name)
{
	std::string path = fmt::format("resources/skyboxes/{}", name);
	
	nlohmann::json root = JsonHelper::loadJsonFromFile(fmt::format("{}/skybox.json", path));
	
	std::string facePaths[6] =
	{
		path + '/' + static_cast<std::string>(root["right"]),
		path + '/' + static_cast<std::string>(root["left"]),
		path + '/' + static_cast<std::string>(root["down"]),
		path + '/' + static_cast<std::string>(root["up"]),
		path + '/' + static_cast<std::string>(root["front"]),
		path + '/' + static_cast<std::string>(root["back"])
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
			throw std::runtime_error(fmt::format("Unable to load image {} from disk", path));
		}
		
		if (firstIteration)
		{
			skyboxComp = skyboxData.data[i].getChannels();
			skyboxSize = skyboxData.data[i].getSize();
			firstIteration = false;
		}
		
		if (skyboxData.data[i].getChannels() != skyboxComp || skyboxData.data[i].getSize() != skyboxSize)
		{
			throw std::runtime_error(fmt::format("Skybox {} have images with different formats", name));
		}
	}
	
	TextureInfo info = TextureHelper::getTextureInfo(skyboxComp, true, true);
	
	skyboxData.pixelFormat = info.pixelFormat;
	skyboxData.internalFormat = info.internalFormat;
	skyboxData.swizzle = info.swizzle;
	
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
