#include "Material.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <format>
#include "Cyph3D/GLObject/Texture.h"
#include "Cyph3D/ResourceManagement/Image.h"

Material* Material::_default;
Material* Material::_missing;

std::map<MaterialMapType, MaterialMapDefinition> Material::_mapDefinitions
{
	{
		MaterialMapType::ALBEDO,
		MaterialMapDefinition{"albedo", ImageType::COLOR_SRGB, {255, 255, 255}}
	},
	{
		MaterialMapType::NORMAL,
		MaterialMapDefinition{"normal", ImageType::NORMAL_MAP, {128, 128}}
	},
	{
		MaterialMapType::ROUGHNESS,
		MaterialMapDefinition{"roughness", ImageType::GRAYSCALE, {128}}
	},
	{
		MaterialMapType::METALNESS,
		MaterialMapDefinition{"metalness", ImageType::GRAYSCALE, {0}}
	},
	{
		MaterialMapType::DISPLACEMENT,
		MaterialMapDefinition{"displacement", ImageType::GRAYSCALE, {255}}
	},
	{
		MaterialMapType::EMISSIVE,
		MaterialMapDefinition{"emissive", ImageType::GRAYSCALE, {0}}
	}
};

Material::Material(std::string name, ResourceManager* resourceManager):
_name(std::move(name))
{
	nlohmann::ordered_json jsonRoot = JsonHelper::loadJsonFromFile(std::format("resources/materials/{}/material.json", _name));
	
	for (const auto& [mapType, mapDefinition] : _mapDefinitions)
	{
		MaterialMap& materialMap = (*_maps.try_emplace(mapType).first).second;
		
		TextureProperties textureProperties = TextureHelper::getTextureProperties(mapDefinition.type);
		
		TextureCreateInfo createInfo;
		createInfo.size = glm::ivec2(1);
		createInfo.internalFormat = textureProperties.internalFormat;
		createInfo.minFilter = GL_NEAREST;
		createInfo.magFilter = GL_NEAREST;
		createInfo.swizzle = textureProperties.swizzle;
		
		materialMap.defaultTexture = std::make_unique<Texture>(createInfo);
		
		PixelProperties pixelProperties = TextureHelper::getPixelProperties(mapDefinition.defaultData.size(), 8);
		materialMap.defaultTexture->setData(mapDefinition.defaultData.data(), pixelProperties.format, pixelProperties.type);
		
		if (jsonRoot.contains(mapDefinition.name))
		{
			materialMap.imageTexture = resourceManager->requestImage(
					std::format("materials/{}/{}", _name, jsonRoot[mapDefinition.name].get<std::string>()),
					mapDefinition.type);
		}
	}
}

void Material::initialize()
{
	_missing = Engine::getGlobalRM().requestMaterial("internal/Missing Material");
	_default = Engine::getGlobalRM().requestMaterial("internal/Default Material");
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
	return _default;
}

Material* Material::getMissing()
{
	return _missing;
}

const Texture& Material::getTexture(MaterialMapType mapType)
{
	MaterialMap& map = _maps.at(mapType);
	
	if (map.defaultTexture && map.imageTexture != nullptr && map.imageTexture->isResourceReady())
	{
		map.defaultTexture.reset();
	}
	
	if (map.imageTexture != nullptr && map.imageTexture->isResourceReady())
	{
		return map.imageTexture->getResource();
	}
	else
	{
		return *map.defaultTexture;
	}
}