#include "Skybox.h"
#include "../Helper/JsonHelper.h"
#include "../Helper/TextureHelper.h"
#include <stb_image.h>
#include <fmt/core.h>

void Skybox::finishLoading(const SkyboxLoadingData& data)
{
	CubemapCreateInfo createInfo;
	createInfo.size = data.size;
	createInfo.internalFormat = data.internalFormat;
	createInfo.textureFiltering = GL_LINEAR;
	createInfo.swizzle = data.swizzle;
	
	_resource = std::make_unique<Cubemap>(createInfo);
	
	for (int i = 0; i < data.data.size(); ++i)
	{
		_resource->setData(data.data[i].get(), i, data.pixelFormat);
	}
}

SkyboxLoadingData Skybox::loadFromFile(const std::string& name)
{
	std::string path = fmt::format("resources/skyboxes/{}", name);
	
	nlohmann::json root = loadJsonFromFile(fmt::format("{}/skybox.json", path));
	
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
	
	int comp;
	
	bool firstIteration = true;
	for (int i = 0; i < 6; ++i)
	{
		glm::ivec2 faceSize;
		int faceComp;
		
		skyboxData.data[i].reset(stbi_load(facePaths[i].c_str(), &faceSize.x, &faceSize.y, &faceComp, 0));
		
		if (skyboxData.data[i] == nullptr)
		{
			throw std::runtime_error(fmt::format("Unable to load image {} from disk", path));
		}
		
		if (firstIteration)
		{
			skyboxData.size = faceSize;
			comp = faceComp;
			firstIteration = false;
		}
		
		if (faceSize != skyboxData.size || faceComp != comp)
		{
			throw std::runtime_error(fmt::format("Skybox {} have images with different formats", name));
		}
	}
	
	TextureInfo info = getTextureInfo(comp, true, true);
	
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
