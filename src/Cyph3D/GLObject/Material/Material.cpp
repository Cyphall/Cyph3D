#include "Material.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/ResourceManagement/Image.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/Helper/FileHelper.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <format>

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

Material::Material(const std::string& path, ResourceManager* resourceManager):
_path(path)
{
	nlohmann::ordered_json jsonRoot = JsonHelper::loadJsonFromFile(FileHelper::getResourcePath() / _path);
	
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
		
		materialMap.defaultTexture = std::make_unique<GLTexture>(createInfo);
		
		PixelProperties pixelProperties = TextureHelper::getPixelProperties(mapDefinition.defaultData.size(), 8);
		materialMap.defaultTexture->setData(mapDefinition.defaultData.data(), 0, pixelProperties.format, pixelProperties.type);
		
		if (jsonRoot.contains(mapDefinition.name))
		{
			materialMap.imageTexture = resourceManager->requestImage(
				jsonRoot[mapDefinition.name].get<std::string>(),
				mapDefinition.type);
		}
	}
}

void Material::initialize()
{
	_missing = Engine::getGlobalRM().requestMaterial("materials/internal/Missing Material/Missing Material.c3dmaterial");
	_default = Engine::getGlobalRM().requestMaterial("materials/internal/Default Material/Default Material.c3dmaterial");
}

const std::string& Material::getPath() const
{
	return _path;
}

Material* Material::getDefault()
{
	return _default;
}

Material* Material::getMissing()
{
	return _missing;
}

const GLTexture& Material::getTexture(MaterialMapType mapType)
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