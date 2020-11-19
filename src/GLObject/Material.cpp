#include "Material.h"
#include "../Engine.h"
#include "../Helper/TextureHelper.h"
#include "../Helper/JsonHelper.h"
#include "../ResourceManagement/ResourceManager.h"
#include "../Logger.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <fmt/core.h>

std::unique_ptr<Material> Material::_default;

Material::Material(std::string name, ResourceManager* resourceManager):
_name(std::move(name))
{
	nlohmann::json jsonRoot = JsonHelper::loadJsonFromFile(fmt::format("resources/materials/{}/material.json", _name));
	
	if (!jsonRoot.contains("shader"))
	{
		throw std::runtime_error(fmt::format("material.json of material {} doesn't contain a \"shader\" entry.", _name));
	}
	
	_shaderProgram = resourceManager->requestMaterialShaderProgram(jsonRoot["shader"]);
	
	for (const auto& [mapName, mapDefinition] : _shaderProgram->getMapDefinitions())
	{
		TextureInfo textureInfo = TextureHelper::getTextureInfo(mapDefinition.defaultData.size(), mapDefinition.compressed, mapDefinition.sRGB);
		
		TextureCreateInfo createInfo;
		createInfo.size = glm::ivec2(1);
		createInfo.internalFormat = textureInfo.internalFormat;
		createInfo.textureFiltering = GL_NEAREST;
		createInfo.swizzle = textureInfo.swizzle;
		
		std::unique_ptr<Texture> defaultColor = std::make_unique<Texture>(createInfo);
		defaultColor->setData(mapDefinition.defaultData.data(), textureInfo.pixelFormat);
		
		Image* image = nullptr;
		
		if (jsonRoot.contains(mapName))
		{
			image = resourceManager->requestImage(
					_name + '/' + static_cast<std::string>(jsonRoot[mapName]),
					mapDefinition.sRGB,
					mapDefinition.compressed);
		}
		
		_textures[mapName] = std::make_tuple(std::move(defaultColor), image);
	}
}

Material::Material():
_shaderProgram(Engine::getGlobalRM().requestMaterialShaderProgram("unlit")), _name("Default Material"), _loaded(true)
{
	TextureInfo textureInfo = TextureHelper::getTextureInfo(3, true, true);
	
	TextureCreateInfo createInfo;
	createInfo.size = glm::ivec2(1);
	createInfo.internalFormat = textureInfo.internalFormat;
	createInfo.textureFiltering = GL_NEAREST;
	createInfo.swizzle = textureInfo.swizzle;
	
	std::unique_ptr<Texture> defaultColor = std::make_unique<Texture>(createInfo);
	
	uint8_t defaultData[] = {255, 0, 255};
	defaultColor->setData(defaultData, textureInfo.pixelFormat);
	
	_textures["colorMap"] = std::make_tuple(std::move(defaultColor), nullptr);
}

void Material::initialize()
{
	_default = std::unique_ptr<Material>(new Material());
}

void Material::bind(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos)
{
	bool allImagesAreReady = true;
	for (auto& [name, data] : _textures)
	{
		auto& [texture, image] = data;
		if (texture && image != nullptr && image->isResourceReady())
		{
			texture.reset();
		}
		else if (image != nullptr && !image->isResourceReady())
		{
			allImagesAreReady = false;
		}
		
		if (image != nullptr && image->isResourceReady())
		{
			_shaderProgram->getShaderProgram()->setUniform(name.c_str(), &image->getResource());
		}
		else
		{
			_shaderProgram->getShaderProgram()->setUniform(name.c_str(), texture.get());
		}
	}
	
	if (!_loaded && allImagesAreReady)
	{
		Logger::Info(fmt::format("Material \"{}\" loaded", _name));
		_loaded = true;
	}
	
	glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));
	_shaderProgram->getShaderProgram()->setUniform("normalMatrix", &normalMatrix);
	_shaderProgram->getShaderProgram()->setUniform("model", &model);
	_shaderProgram->getShaderProgram()->setUniform("view", &view);
	_shaderProgram->getShaderProgram()->setUniform("projection", &projection);
	_shaderProgram->getShaderProgram()->setUniform("viewPos", &cameraPos);
	
	_shaderProgram->getShaderProgram()->bind();
}

const std::string& Material::getName() const
{
	return _name;
}

void Material::setName(std::string name)
{
	_name = std::move(name);
}

Material* Material::getDefault()
{
	return _default.get();
}
