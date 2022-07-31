#include "Skybox.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/GLObject/CreateInfo/CubemapCreateInfo.h"
#include <format>
#include <chrono>
#include "Cyph3D/ResourceManagement/StbImage.h"

float Skybox::getRotation() const
{
	return _rotation;
}

void Skybox::setRotation(float rotation)
{
	_rotation = rotation;
}

void Skybox::loadResourceImpl()
{
	std::string path = std::format("resources/skyboxes/{}", _name);
	
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
	
	std::array<StbImage, 6> data;
	int skyboxComp;
	glm::ivec2 skyboxSize;
	
	bool firstIteration = true;
	for (int i = 0; i < 6; ++i)
	{
		data[i] = StbImage(facePaths[i].c_str());
		
		if (!data[i].isValid())
		{
			throw std::runtime_error(std::format("Unable to load image {} from disk", path));
		}
		
		if (firstIteration)
		{
			skyboxComp = data[i].getChannelCount();
			skyboxSize = data[i].getSize();
			firstIteration = false;
		}
		
		if (data[i].getChannelCount() != skyboxComp || data[i].getSize() != skyboxSize)
		{
			throw std::runtime_error(std::format("Skybox {} have images with different formats", _name));
		}
	}
	
	TextureProperties textureProperties = TextureHelper::getTextureProperties(COLOR_SRGB);
	
	CubemapCreateInfo createInfo;
	createInfo.size = data[0].getSize();
	createInfo.internalFormat = textureProperties.internalFormat;
	createInfo.textureFiltering = GL_LINEAR;
	createInfo.swizzle = textureProperties.swizzle;
	
	_resource = std::make_unique<Cubemap>(createInfo);
	
	for (int i = 0; i < data.size(); i++)
	{
		PixelProperties pixelProperties = TextureHelper::getPixelProperties(data[i].getChannelCount(), data[i].getBitPerChannel());
		
		_resource->setData(data[i].getPtr(), i, pixelProperties.format, pixelProperties.type);
	}
	
	GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	auto timeout = std::chrono::seconds(10);
	glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count());
}