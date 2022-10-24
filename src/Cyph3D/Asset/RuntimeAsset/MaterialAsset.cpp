#include "MaterialAsset.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"

#include <glm/gtc/type_ptr.hpp>

MaterialAsset* MaterialAsset::_defaultMaterial = nullptr;
MaterialAsset* MaterialAsset::_missingMaterial = nullptr;

MaterialAsset::MaterialAsset(AssetManager& manager, const MaterialAssetSignature& signature):
	RuntimeAsset(manager, signature)
{
	nlohmann::ordered_json jsonRoot = JsonHelper::loadJsonFromFile(FileHelper::getAssetDirectoryPath() / signature.path);

	int version;
	auto versionIt = jsonRoot.find("version");
	if (versionIt != jsonRoot.end())
	{
		version = versionIt.value().get<int>();
	}
	else
	{
		version = 1;
	}

	switch (version)
	{
		case 1:
			deserializeFromVersion1(jsonRoot);
			break;
		case 2:
			deserializeFromVersion2(jsonRoot);
			break;
		default:
			throw;
	}
}

MaterialAsset::~MaterialAsset()
{}

bool MaterialAsset::isLoaded() const
{
	return true;
}

const std::string& MaterialAsset::getPath() const
{
	return _signature.path;
}

const std::string* MaterialAsset::getAlbedoMapPath() const
{
	return _albedoMapPath.has_value() ? &_albedoMapPath.value() : nullptr;
}

void MaterialAsset::setAlbedoMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_albedoMapPath = *path;
		_albedoMap = _manager.loadTexture(path.value(), TextureType::ColorSrgb);
		_albedoValueTexture = nullptr;
	}
	else
	{
		_albedoMapPath = std::nullopt;
		_albedoMap = nullptr;
		
		TextureCreateInfo createInfo;
		createInfo.size = {1, 1};
		createInfo.internalFormat = GL_RGB8;

		_albedoValueTexture = std::make_unique<GLTexture>(createInfo);
		setAlbedoValue(getAlbedoValue()); // updates the texture with the current value
	}
}

const GLTexture& MaterialAsset::getAlbedoTexture() const
{
	return _albedoMap != nullptr && _albedoMap->isLoaded() ? _albedoMap->getGLTexture() : *_albedoValueTexture;
}

const std::string* MaterialAsset::getNormalMapPath() const
{
	return _normalMapPath.has_value() ? &_normalMapPath.value() : nullptr;
}

void MaterialAsset::setNormalMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_normalMapPath = *path;
		_normalMap = _manager.loadTexture(*path, TextureType::NormalMap);
		_normalValueTexture = nullptr;
	}
	else
	{
		_normalMapPath = std::nullopt;
		_normalMap = nullptr;

		TextureCreateInfo createInfo;
		createInfo.size = {1, 1};
		createInfo.internalFormat = GL_RG8;

		_normalValueTexture = std::make_unique<GLTexture>(createInfo);
		
		float normalValue[] = {0.5f, 0.5f};
		_normalValueTexture->setData(normalValue, 0, GL_RG, GL_FLOAT);
	}
}

const GLTexture& MaterialAsset::getNormalTexture() const
{
	return _normalMap != nullptr && _normalMap->isLoaded() ? _normalMap->getGLTexture() : *_normalValueTexture;
}

const std::string* MaterialAsset::getRoughnessMapPath() const
{
	return _roughnessMapPath.has_value() ? &_roughnessMapPath.value() : nullptr;
}

void MaterialAsset::setRoughnessMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_roughnessMapPath = *path;
		_roughnessMap = _manager.loadTexture(*path, TextureType::Grayscale);
		_roughnessValueTexture = nullptr;
	}
	else
	{
		_roughnessMapPath = std::nullopt;
		_roughnessMap = nullptr;

		TextureCreateInfo createInfo;
		createInfo.size = {1, 1};
		createInfo.internalFormat = GL_R8;

		_roughnessValueTexture = std::make_unique<GLTexture>(createInfo);
		setRoughnessValue(getRoughnessValue()); // updates the texture with the current value
	}
}

const GLTexture& MaterialAsset::getRoughnessTexture() const
{
	return _roughnessMap != nullptr && _roughnessMap->isLoaded() ? _roughnessMap->getGLTexture() : *_roughnessValueTexture;
}

const std::string* MaterialAsset::getMetalnessMapPath() const
{
	return _metalnessMapPath.has_value() ? &_metalnessMapPath.value() : nullptr;
}

void MaterialAsset::setMetalnessMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_metalnessMapPath = *path;
		_metalnessMap = _manager.loadTexture(*path, TextureType::Grayscale);
		_metalnessValueTexture = nullptr;
	}
	else
	{
		_metalnessMapPath = std::nullopt;
		_metalnessMap = nullptr;

		TextureCreateInfo createInfo;
		createInfo.size = {1, 1};
		createInfo.internalFormat = GL_R8;

		_metalnessValueTexture = std::make_unique<GLTexture>(createInfo);
		setMetalnessValue(getMetalnessValue()); // updates the texture with the current value
	}
}

const GLTexture& MaterialAsset::getMetalnessTexture() const
{
	return _metalnessMap != nullptr && _metalnessMap->isLoaded() ? _metalnessMap->getGLTexture() : *_metalnessValueTexture;
}

const std::string* MaterialAsset::getDisplacementMapPath() const
{
	return _displacementMapPath.has_value() ? &_displacementMapPath.value() : nullptr;
}

void MaterialAsset::setDisplacementMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_displacementMapPath = *path;
		_displacementMap = _manager.loadTexture(*path, TextureType::Grayscale);
		_displacementValueTexture = nullptr;
	}
	else
	{
		_displacementMapPath = std::nullopt;
		_displacementMap = nullptr;

		TextureCreateInfo createInfo;
		createInfo.size = {1, 1};
		createInfo.internalFormat = GL_R8;

		_displacementValueTexture = std::make_unique<GLTexture>(createInfo);

		float displacementValue[] = {1.0f};
		_displacementValueTexture->setData(displacementValue, 0, GL_RED, GL_FLOAT);
	}
}

const GLTexture& MaterialAsset::getDisplacementTexture() const
{
	return _displacementMap != nullptr && _displacementMap->isLoaded() ? _displacementMap->getGLTexture() : *_displacementValueTexture;
}

const std::string* MaterialAsset::getEmissiveMapPath() const
{
	return _emissiveMapPath.has_value() ? &_emissiveMapPath.value() : nullptr;
}

void MaterialAsset::setEmissiveMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_emissiveMapPath = *path;
		_emissiveMap = _manager.loadTexture(*path, TextureType::Grayscale);
		_emissiveValueTexture = nullptr;
	}
	else
	{
		_emissiveMapPath = std::nullopt;
		_emissiveMap = nullptr;

		TextureCreateInfo createInfo;
		createInfo.size = {1, 1};
		createInfo.internalFormat = GL_R8;

		_emissiveValueTexture = std::make_unique<GLTexture>(createInfo);
		setEmissiveValue(getEmissiveValue()); // updates the texture with the current value
	}
}

const GLTexture& MaterialAsset::getEmissiveTexture() const
{
	return _emissiveMap != nullptr && _emissiveMap->isLoaded() ? _emissiveMap->getGLTexture() : *_emissiveValueTexture;
}

const glm::vec3& MaterialAsset::getAlbedoValue() const
{
	return _albedoValue;
}

void MaterialAsset::setAlbedoValue(const glm::vec3& value)
{
	_albedoValue = value;
	
	if (_albedoValueTexture)
	{
		_albedoValueTexture->setData(glm::value_ptr(_albedoValue), 0, GL_RGB, GL_FLOAT);
	}
}

const float& MaterialAsset::getRoughnessValue() const
{
	return _roughnessValue;
}

void MaterialAsset::setRoughnessValue(const float& value)
{
	_roughnessValue = value;

	if (_roughnessValueTexture)
	{
		_roughnessValueTexture->setData(&_roughnessValue, 0, GL_RED, GL_FLOAT);
	}
}

const float& MaterialAsset::getMetalnessValue() const
{
	return _metalnessValue;
}

void MaterialAsset::setMetalnessValue(const float& value)
{
	_metalnessValue = value;

	if (_metalnessValueTexture)
	{
		_metalnessValueTexture->setData(&_metalnessValue, 0, GL_RED, GL_FLOAT);
	}
}

const float& MaterialAsset::getEmissiveValue() const
{
	return _emissiveValue;
}

void MaterialAsset::setEmissiveValue(const float& value)
{
	_emissiveValue = value;

	if (_emissiveValueTexture)
	{
		_emissiveValueTexture->setData(&_emissiveValue, 0, GL_RED, GL_FLOAT);
	}
}

void MaterialAsset::initialize()
{
	_defaultMaterial = Engine::getAssetManager().loadMaterial("materials/internal/Default Material/Default Material.c3dmaterial");
	_missingMaterial = Engine::getAssetManager().loadMaterial("materials/internal/Missing Material/Missing Material.c3dmaterial");
}

MaterialAsset* MaterialAsset::getDefaultMaterial()
{
	return _defaultMaterial;
}

MaterialAsset* MaterialAsset::getMissingMaterial()
{
	return _missingMaterial;
}

void MaterialAsset::deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot)
{
	{
		auto jsonIt = jsonRoot.find("albedo");
		if (jsonIt != jsonRoot.end())
		{
			setAlbedoMapPath(jsonIt.value().get<std::string>());
		}
		else
		{
			setAlbedoMapPath(std::nullopt);
		}

		setAlbedoValue({1, 1, 1});
	}

	{
		auto jsonIt = jsonRoot.find("normal");
		if (jsonIt != jsonRoot.end())
		{
			setNormalMapPath(jsonIt.value().get<std::string>());
		}
		else
		{
			setNormalMapPath(std::nullopt);
		}
	}

	{
		auto jsonIt = jsonRoot.find("roughness");
		if (jsonIt != jsonRoot.end())
		{
			setRoughnessMapPath(jsonIt.value().get<std::string>());
		}
		else
		{
			setRoughnessMapPath(std::nullopt);
		}

		setRoughnessValue(0.5f);
	}

	{
		auto jsonIt = jsonRoot.find("metalness");
		if (jsonIt != jsonRoot.end())
		{
			setMetalnessMapPath(jsonIt.value().get<std::string>());
		}
		else
		{
			setMetalnessMapPath(std::nullopt);
		}

		setMetalnessValue(0.0f);
	}

	{
		auto jsonIt = jsonRoot.find("displacement");
		if (jsonIt != jsonRoot.end())
		{
			setDisplacementMapPath(jsonIt.value().get<std::string>());
		}
		else
		{
			setDisplacementMapPath(std::nullopt);
		}
	}

	{
		auto jsonIt = jsonRoot.find("emissive");
		if (jsonIt != jsonRoot.end())
		{
			setEmissiveMapPath(jsonIt.value().get<std::string>());
		}
		else
		{
			setEmissiveMapPath(std::nullopt);
		}

		setEmissiveValue(0.0f);
	}
}

void MaterialAsset::deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot)
{
	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("albedo");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setAlbedoMapPath(jsonPath.get<std::string>());
		}
		else
		{
			setAlbedoMapPath(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setAlbedoValue({
			jsonValue.at(0).get<float>(),
			jsonValue.at(1).get<float>(),
			jsonValue.at(2).get<float>()
		});
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("normal");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setNormalMapPath(jsonPath.get<std::string>());
		}
		else
		{
			setNormalMapPath(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("roughness");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setRoughnessMapPath(jsonPath.get<std::string>());
		}
		else
		{
			setRoughnessMapPath(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setRoughnessValue(jsonValue.get<float>());
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("metalness");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setMetalnessMapPath(jsonPath.get<std::string>());
		}
		else
		{
			setMetalnessMapPath(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setMetalnessValue(jsonValue.get<float>());
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("displacement");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setDisplacementMapPath(jsonPath.get<std::string>());
		}
		else
		{
			setDisplacementMapPath(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("emissive");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setEmissiveMapPath(jsonPath.get<std::string>());
		}
		else
		{
			setEmissiveMapPath(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setEmissiveValue(jsonValue.get<float>());
	}
}