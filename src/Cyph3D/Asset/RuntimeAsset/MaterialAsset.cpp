#include "MaterialAsset.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Helper/ImGuiHelper.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/Window.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

MaterialAsset* MaterialAsset::_defaultMaterial = nullptr;
MaterialAsset* MaterialAsset::_missingMaterial = nullptr;

MaterialAsset::MaterialAsset(AssetManager& manager, const MaterialAssetSignature& signature):
	RuntimeAsset(manager, signature)
{
	reload();
}

MaterialAsset::~MaterialAsset()
{}

bool MaterialAsset::isLoaded() const
{
	return (_albedoTexture != nullptr ? _albedoTexture->isLoaded() : true) &&
	       (_normalTexture != nullptr ? _normalTexture->isLoaded() : true) &&
	       (_roughnessTexture != nullptr ? _roughnessTexture->isLoaded() : true) &&
	       (_metalnessTexture != nullptr ? _metalnessTexture->isLoaded() : true) &&
	       (_displacementTexture != nullptr ? _displacementTexture->isLoaded() : true) &&
	       (_emissiveTexture != nullptr ? _emissiveTexture->isLoaded() : true);
}

void MaterialAsset::onDrawUi()
{
	ImGuiHelper::TextCentered("Material");
	ImGuiHelper::TextCentered(_signature.path.c_str());

	ImGui::Separator();

	if (ImGui::Button("Reload"))
	{
		reload();
	}

	ImGui::SameLine();

	if (ImGui::Button("Save"))
	{
		save();
	}

	ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
	{
		ImGuiHelper::BeginGroupPanel("Albedo");

		std::optional<std::string_view> newPath;
		if (ImGuiHelper::AssetInputWidget(_albedoTexture ? &_albedoTexture->getSignature().path : nullptr, "Image", "asset_image", newPath))
		{
			setAlbedoTexture(newPath);
		}

		if (_albedoTexture == nullptr)
		{
			glm::vec3 value = getAlbedoValue();
			if (ImGui::ColorEdit3("Value", glm::value_ptr(value), ImGuiColorEditFlags_Float))
			{
				setAlbedoValue(value);
			}
		}

		ImGuiHelper::EndGroupPanel();
	}
	ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
	{
		ImGuiHelper::BeginGroupPanel("Normal");

		std::optional<std::string_view> newPath;
		if (ImGuiHelper::AssetInputWidget(_normalTexture ? &_normalTexture->getSignature().path : nullptr, "Image", "asset_image", newPath))
		{
			setNormalTexture(newPath);
		}

		ImGuiHelper::EndGroupPanel();
	}
	ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
	{
		ImGuiHelper::BeginGroupPanel("Roughness");

		std::optional<std::string_view> newPath;
		if (ImGuiHelper::AssetInputWidget(_roughnessTexture ? &_roughnessTexture->getSignature().path : nullptr, "Image", "asset_image", newPath))
		{
			setRoughnessTexture(newPath);
		}

		if (_roughnessTexture == nullptr)
		{
			float value = getRoughnessValue();
			if (ImGui::SliderFloat("Value", &value, 0.0f, 1.0f))
			{
				setRoughnessValue(value);
			}
		}

		ImGuiHelper::EndGroupPanel();
	}
	ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
	{
		ImGuiHelper::BeginGroupPanel("Metalness");

		std::optional<std::string_view> newPath;
		if (ImGuiHelper::AssetInputWidget(_metalnessTexture ? &_metalnessTexture->getSignature().path : nullptr, "Image", "asset_image", newPath))
		{
			setMetalnessTexture(newPath);
		}

		if (_metalnessTexture == nullptr)
		{
			float value = getMetalnessValue();
			if (ImGui::SliderFloat("Value", &value, 0.0f, 1.0f))
			{
				setMetalnessValue(value);
			}
		}

		ImGuiHelper::EndGroupPanel();
	}
	ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
	{
		ImGuiHelper::BeginGroupPanel("Displacement");

		std::optional<std::string_view> newPath;
		if (ImGuiHelper::AssetInputWidget(_displacementTexture ? &_displacementTexture->getSignature().path : nullptr, "Image", "asset_image", newPath))
		{
			setDisplacementTexture(newPath);
		}

		float scale = getDisplacementScale();
		if (ImGui::DragFloat("Scale", &scale, 0.001f, 0.0f, FLT_MAX, "%.3f"))
		{
			setDisplacementScale(scale);
		}

		ImGuiHelper::EndGroupPanel();
	}
	ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
	{
		ImGuiHelper::BeginGroupPanel("Emissive");

		std::optional<std::string_view> newPath;
		if (ImGuiHelper::AssetInputWidget(_emissiveTexture ? &_emissiveTexture->getSignature().path : nullptr, "Image", "asset_image", newPath))
		{
			setEmissiveTexture(newPath);
		}

		float scale = getEmissiveScale();
		if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.0f, FLT_MAX, "%.2f"))
		{
			setEmissiveScale(scale);
		}

		ImGuiHelper::EndGroupPanel();
	}
}

void MaterialAsset::setAlbedoTexture(std::optional<std::string_view> path)
{
	if (path)
	{
		_albedoTexture = _manager.loadTexture(path.value(), ImageType::ColorSrgb);
		_albedoTextureChangedConnection = _albedoTexture->getChangedSignal().connect(
			[this]()
			{
				_changed();
			}
		);
	}
	else
	{
		_albedoTexture = nullptr;
		_albedoTextureChangedConnection = {};
	}

	_changed();
}

int32_t MaterialAsset::getAlbedoTextureBindlessIndex() const
{
	return _albedoTexture != nullptr && _albedoTexture->isLoaded() ? _albedoTexture->getBindlessIndex() : -1;
}

void MaterialAsset::setNormalTexture(std::optional<std::string_view> path)
{
	if (path)
	{
		_normalTexture = _manager.loadTexture(*path, ImageType::NormalMap);
		_normalTextureChangedConnection = _normalTexture->getChangedSignal().connect(
			[this]()
			{
				_changed();
			}
		);
	}
	else
	{
		_normalTexture = nullptr;
		_normalTextureChangedConnection = {};
	}

	_changed();
}

int32_t MaterialAsset::getNormalTextureBindlessIndex() const
{
	return _normalTexture != nullptr && _normalTexture->isLoaded() ? _normalTexture->getBindlessIndex() : -1;
}

void MaterialAsset::setRoughnessTexture(std::optional<std::string_view> path)
{
	if (path)
	{
		_roughnessTexture = _manager.loadTexture(*path, ImageType::Grayscale);
		_roughnessTextureChangedConnection = _roughnessTexture->getChangedSignal().connect(
			[this]()
			{
				_changed();
			}
		);
	}
	else
	{
		_roughnessTexture = nullptr;
		_roughnessTextureChangedConnection = {};
	}

	_changed();
}

int32_t MaterialAsset::getRoughnessTextureBindlessIndex() const
{
	return _roughnessTexture != nullptr && _roughnessTexture->isLoaded() ? _roughnessTexture->getBindlessIndex() : -1;
}

void MaterialAsset::setMetalnessTexture(std::optional<std::string_view> path)
{
	if (path)
	{
		_metalnessTexture = _manager.loadTexture(*path, ImageType::Grayscale);
		_metalnessTextureChangedConnection = _metalnessTexture->getChangedSignal().connect(
			[this]()
			{
				_changed();
			}
		);
	}
	else
	{
		_metalnessTexture = nullptr;
		_metalnessTextureChangedConnection = {};
	}

	_changed();
}

int32_t MaterialAsset::getMetalnessTextureBindlessIndex() const
{
	return _metalnessTexture != nullptr && _metalnessTexture->isLoaded() ? _metalnessTexture->getBindlessIndex() : -1;
}

void MaterialAsset::setDisplacementTexture(std::optional<std::string_view> path)
{
	if (path)
	{
		_displacementTexture = _manager.loadTexture(*path, ImageType::Grayscale);
		_displacementTextureChangedConnection = _displacementTexture->getChangedSignal().connect(
			[this]()
			{
				_changed();
			}
		);
	}
	else
	{
		_displacementTexture = nullptr;
		_displacementTextureChangedConnection = {};
	}

	_changed();
}

int32_t MaterialAsset::getDisplacementTextureBindlessIndex() const
{
	return _displacementTexture != nullptr && _displacementTexture->isLoaded() ? _displacementTexture->getBindlessIndex() : -1;
}

void MaterialAsset::setEmissiveTexture(std::optional<std::string_view> path)
{
	if (path)
	{
		_emissiveTexture = _manager.loadTexture(*path, ImageType::Grayscale);
		_emissiveTextureChangedConnection = _emissiveTexture->getChangedSignal().connect(
			[this]()
			{
				_changed();
			}
		);
	}
	else
	{
		_emissiveTexture = nullptr;
		_emissiveTextureChangedConnection = {};
	}

	_changed();
}

int32_t MaterialAsset::getEmissiveTextureBindlessIndex() const
{
	return _emissiveTexture != nullptr && _emissiveTexture->isLoaded() ? _emissiveTexture->getBindlessIndex() : -1;
}

const glm::vec3& MaterialAsset::getAlbedoValue() const
{
	return _albedoValue;
}

void MaterialAsset::setAlbedoValue(const glm::vec3& value)
{
	_albedoValue = glm::clamp(value, glm::vec3(0.0f), glm::vec3(1.0f));

	_changed();
}

const float& MaterialAsset::getRoughnessValue() const
{
	return _roughnessValue;
}

void MaterialAsset::setRoughnessValue(const float& value)
{
	_roughnessValue = glm::clamp(value, 0.0f, 1.0f);

	_changed();
}

const float& MaterialAsset::getMetalnessValue() const
{
	return _metalnessValue;
}

void MaterialAsset::setMetalnessValue(const float& value)
{
	_metalnessValue = glm::clamp(value, 0.0f, 1.0f);

	_changed();
}

const float& MaterialAsset::getDisplacementScale() const
{
	return _displacementScale;
}

void MaterialAsset::setDisplacementScale(const float& scale)
{
	_displacementScale = glm::max(scale, 0.0f);

	_changed();
}

const float& MaterialAsset::getEmissiveScale() const
{
	return _emissiveScale;
}

void MaterialAsset::setEmissiveScale(const float& scale)
{
	_emissiveScale = glm::max(scale, 0.0f);

	_changed();
}

void MaterialAsset::initDefaultAndMissing()
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

void MaterialAsset::create(std::string_view path)
{
	nlohmann::ordered_json jsonRoot;
	jsonRoot["version"] = 2;

	{
		nlohmann::ordered_json jsonMap;
		jsonMap["path"] = nullptr;
		jsonMap["value"] = {1.0f, 1.0f, 1.0f};
		jsonRoot["albedo"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		jsonMap["path"] = nullptr;
		jsonRoot["normal"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		jsonMap["path"] = nullptr;
		jsonMap["value"] = 0.5f;
		jsonRoot["roughness"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		jsonMap["path"] = nullptr;
		jsonMap["value"] = 0.0f;
		jsonRoot["metalness"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		jsonMap["path"] = nullptr;
		jsonRoot["displacement"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		jsonMap["path"] = nullptr;
		jsonMap["value"] = 0.0f;
		jsonRoot["emissive"] = jsonMap;
	}

	JsonHelper::saveJsonToFile(jsonRoot, FileHelper::getAssetDirectoryPath() / path);
}

void MaterialAsset::deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot)
{
	{
		auto jsonIt = jsonRoot.find("albedo");
		if (jsonIt != jsonRoot.end())
		{
			setAlbedoTexture(jsonIt.value().get<std::string>());
		}
		else
		{
			setAlbedoTexture(std::nullopt);
		}

		setAlbedoValue({1, 1, 1});
	}

	{
		auto jsonIt = jsonRoot.find("normal");
		if (jsonIt != jsonRoot.end())
		{
			setNormalTexture(jsonIt.value().get<std::string>());
		}
		else
		{
			setNormalTexture(std::nullopt);
		}
	}

	{
		auto jsonIt = jsonRoot.find("roughness");
		if (jsonIt != jsonRoot.end())
		{
			setRoughnessTexture(jsonIt.value().get<std::string>());
		}
		else
		{
			setRoughnessTexture(std::nullopt);
		}

		setRoughnessValue(0.5f);
	}

	{
		auto jsonIt = jsonRoot.find("metalness");
		if (jsonIt != jsonRoot.end())
		{
			setMetalnessTexture(jsonIt.value().get<std::string>());
		}
		else
		{
			setMetalnessTexture(std::nullopt);
		}

		setMetalnessValue(0.0f);
	}

	{
		auto jsonIt = jsonRoot.find("displacement");
		if (jsonIt != jsonRoot.end())
		{
			setDisplacementTexture(jsonIt.value().get<std::string>());
		}
		else
		{
			setDisplacementTexture(std::nullopt);
		}

		setDisplacementScale(0.05f);
	}

	{
		auto jsonIt = jsonRoot.find("emissive");
		if (jsonIt != jsonRoot.end())
		{
			setEmissiveTexture(jsonIt.value().get<std::string>());
		}
		else
		{
			setEmissiveTexture(std::nullopt);
		}

		setEmissiveScale(0.0f);
	}
}

void MaterialAsset::deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot)
{
	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("albedo");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setAlbedoTexture(jsonPath.get<std::string>());
		}
		else
		{
			setAlbedoTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setAlbedoValue({
			jsonValue.at(0).get<float>(),
			jsonValue.at(1).get<float>(),
			jsonValue.at(2).get<float>(),
		});
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("normal");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setNormalTexture(jsonPath.get<std::string>());
		}
		else
		{
			setNormalTexture(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("roughness");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setRoughnessTexture(jsonPath.get<std::string>());
		}
		else
		{
			setRoughnessTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setRoughnessValue(jsonValue.get<float>());
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("metalness");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setMetalnessTexture(jsonPath.get<std::string>());
		}
		else
		{
			setMetalnessTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setMetalnessValue(jsonValue.get<float>());
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("displacement");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setDisplacementTexture(jsonPath.get<std::string>());
		}
		else
		{
			setDisplacementTexture(std::nullopt);
		}

		setDisplacementScale(0.05f);
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("emissive");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setEmissiveTexture(jsonPath.get<std::string>());

			setEmissiveScale(1.0f);
		}
		else
		{
			setEmissiveTexture(std::nullopt);

			const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
			setEmissiveScale(jsonValue.get<float>());
		}
	}
}

void MaterialAsset::deserializeFromVersion3(const nlohmann::ordered_json& jsonRoot)
{
	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("albedo");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setAlbedoTexture(jsonPath.get<std::string>());
		}
		else
		{
			setAlbedoTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setAlbedoValue({
			jsonValue.at(0).get<float>(),
			jsonValue.at(1).get<float>(),
			jsonValue.at(2).get<float>(),
		});
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("normal");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setNormalTexture(jsonPath.get<std::string>());
		}
		else
		{
			setNormalTexture(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("roughness");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setRoughnessTexture(jsonPath.get<std::string>());
		}
		else
		{
			setRoughnessTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setRoughnessValue(jsonValue.get<float>());
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("metalness");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setMetalnessTexture(jsonPath.get<std::string>());
		}
		else
		{
			setMetalnessTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setMetalnessValue(jsonValue.get<float>());
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("displacement");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setDisplacementTexture(jsonPath.get<std::string>());
		}
		else
		{
			setDisplacementTexture(std::nullopt);
		}

		setDisplacementScale(0.05f);
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("emissive");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setEmissiveTexture(jsonPath.get<std::string>());
		}
		else
		{
			setEmissiveTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("scale");
		setEmissiveScale(jsonValue.get<float>());
	}
}

void MaterialAsset::deserializeFromVersion4(const nlohmann::ordered_json& jsonRoot)
{
	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("albedo");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setAlbedoTexture(jsonPath.get<std::string>());
		}
		else
		{
			setAlbedoTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setAlbedoValue({
			jsonValue.at(0).get<float>(),
			jsonValue.at(1).get<float>(),
			jsonValue.at(2).get<float>(),
		});
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("normal");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setNormalTexture(jsonPath.get<std::string>());
		}
		else
		{
			setNormalTexture(std::nullopt);
		}
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("roughness");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setRoughnessTexture(jsonPath.get<std::string>());
		}
		else
		{
			setRoughnessTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setRoughnessValue(jsonValue.get<float>());
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("metalness");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setMetalnessTexture(jsonPath.get<std::string>());
		}
		else
		{
			setMetalnessTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("value");
		setMetalnessValue(jsonValue.get<float>());
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("displacement");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setDisplacementTexture(jsonPath.get<std::string>());
		}
		else
		{
			setDisplacementTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("scale");
		setDisplacementScale(jsonValue.get<float>());
	}

	{
		const nlohmann::ordered_json& jsonMap = jsonRoot.at("emissive");

		const nlohmann::ordered_json& jsonPath = jsonMap["path"];
		if (!jsonPath.is_null())
		{
			setEmissiveTexture(jsonPath.get<std::string>());
		}
		else
		{
			setEmissiveTexture(std::nullopt);
		}

		const nlohmann::ordered_json& jsonValue = jsonMap.at("scale");
		setEmissiveScale(jsonValue.get<float>());
	}
}

void MaterialAsset::save() const
{
	nlohmann::ordered_json jsonRoot;
	jsonRoot["version"] = 4;

	{
		nlohmann::ordered_json jsonMap;
		if (_albedoTexture)
		{
			jsonMap["path"] = _albedoTexture->getSignature().path;
		}
		else
		{
			jsonMap["path"] = nullptr;
		}

		const glm::vec3& value = getAlbedoValue();
		jsonMap["value"] = {value.x, value.y, value.z};

		jsonRoot["albedo"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		if (_normalTexture)
		{
			jsonMap["path"] = _normalTexture->getSignature().path;
		}
		else
		{
			jsonMap["path"] = nullptr;
		}

		jsonRoot["normal"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		if (_roughnessTexture)
		{
			jsonMap["path"] = _roughnessTexture->getSignature().path;
		}
		else
		{
			jsonMap["path"] = nullptr;
		}

		const float& value = getRoughnessValue();
		jsonMap["value"] = value;

		jsonRoot["roughness"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		if (_metalnessTexture)
		{
			jsonMap["path"] = _metalnessTexture->getSignature().path;
		}
		else
		{
			jsonMap["path"] = nullptr;
		}

		const float& value = getMetalnessValue();
		jsonMap["value"] = value;

		jsonRoot["metalness"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		if (_displacementTexture)
		{
			jsonMap["path"] = _displacementTexture->getSignature().path;
		}
		else
		{
			jsonMap["path"] = nullptr;
		}

		const float& scale = getDisplacementScale();
		jsonMap["scale"] = scale;

		jsonRoot["displacement"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		if (_emissiveTexture)
		{
			jsonMap["path"] = _emissiveTexture->getSignature().path;
		}
		else
		{
			jsonMap["path"] = nullptr;
		}

		const float& scale = getEmissiveScale();
		jsonMap["scale"] = scale;

		jsonRoot["emissive"] = jsonMap;
	}

	JsonHelper::saveJsonToFile(jsonRoot, FileHelper::getAssetDirectoryPath() / _signature.path);
}

void MaterialAsset::reload()
{
	nlohmann::ordered_json jsonRoot = JsonHelper::loadJsonFromFile(FileHelper::getAssetDirectoryPath() / _signature.path);

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
	case 3:
		deserializeFromVersion3(jsonRoot);
		break;
	case 4:
		deserializeFromVersion4(jsonRoot);
		break;
	default:
		throw;
	}
}