#include "MaterialAsset.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Window.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/Helper/ImGuiHelper.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

MaterialAsset* MaterialAsset::_defaultMaterial = nullptr;
MaterialAsset* MaterialAsset::_missingMaterial = nullptr;

template<typename T>
static void updateImageData(const VKPtr<VKImage>& image, const T& value)
{
	VKPtr<VKBuffer<T>> stagingBuffer = VKBuffer<T>::create(
		Engine::getVKContext(),
		1,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);
	
	T* ptr = stagingBuffer->map();
	std::copy(&value, &value + 1, ptr);
	stagingBuffer->unmap();
	
	Engine::getVKContext().executeImmediate(
		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
		{
			commandBuffer->imageMemoryBarrier(
				image,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eTransferDstOptimal,
				0,
				0);
			
			commandBuffer->copyBufferToImage(stagingBuffer, 0, image, 0, 0);
			
			commandBuffer->imageMemoryBarrier(
				image,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal,
				0,
				0);
		});
}

MaterialAsset::MaterialAsset(AssetManager& manager, const MaterialAssetSignature& signature):
	RuntimeAsset(manager, signature)
{
	reload();
}

MaterialAsset::~MaterialAsset()
{}

bool MaterialAsset::isLoaded() const
{
	return ((_albedoMap != nullptr && _albedoMap->isLoaded()) || _albedoValueTextureView) &&
	       ((_normalMap != nullptr && _normalMap->isLoaded()) || _normalValueTextureView) &&
	       ((_roughnessMap != nullptr && _roughnessMap->isLoaded()) || _roughnessValueTextureView) &&
	       ((_metalnessMap != nullptr && _metalnessMap->isLoaded()) || _metalnessValueTextureView) &&
	       ((_displacementMap != nullptr && _displacementMap->isLoaded()) || _displacementValueTextureView) &&
	       ((_emissiveMap != nullptr && _emissiveMap->isLoaded()) || _emissiveValueTextureView);
}

void MaterialAsset::onDrawUi()
{
	ImGuiHelper::TextCentered("Material");
	ImGuiHelper::TextCentered(_signature.path.c_str());
	
	ImGui::Separator();
	
	if (ImGui::Button("Reset"))
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
		if (ImGuiHelper::AssetInputWidget(getAlbedoMapPath(), "Image", "asset_image", newPath))
		{
			setAlbedoMapPath(newPath);
		}
		
		if (_albedoMap == nullptr)
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
		if (ImGuiHelper::AssetInputWidget(getNormalMapPath(), "Image", "asset_image", newPath))
		{
			setNormalMapPath(newPath);
		}

		ImGuiHelper::EndGroupPanel();
	}
	ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
	{
		ImGuiHelper::BeginGroupPanel("Roughness");

		std::optional<std::string_view> newPath;
		if (ImGuiHelper::AssetInputWidget(getRoughnessMapPath(), "Image", "asset_image", newPath))
		{
			setRoughnessMapPath(newPath);
		}

		if (_roughnessMap == nullptr)
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
		if (ImGuiHelper::AssetInputWidget(getMetalnessMapPath(), "Image", "asset_image", newPath))
		{
			setMetalnessMapPath(newPath);
		}

		if (_metalnessMap == nullptr)
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
		if (ImGuiHelper::AssetInputWidget(getDisplacementMapPath(), "Image", "asset_image", newPath))
		{
			setDisplacementMapPath(newPath);
		}

		ImGuiHelper::EndGroupPanel();
	}
	ImGui::Dummy({0, 10.0f * Engine::getWindow().getPixelScale()});
	{
		ImGuiHelper::BeginGroupPanel("Emissive");

		std::optional<std::string_view> newPath;
		if (ImGuiHelper::AssetInputWidget(getEmissiveMapPath(), "Image", "asset_image", newPath))
		{
			setEmissiveMapPath(newPath);
		}

		if (_emissiveMap == nullptr)
		{
			float value = getEmissiveValue();
			if (ImGui::SliderFloat("Value", &value, 0.0f, 1.0f))
			{
				setEmissiveValue(value);
			}
		}

		ImGuiHelper::EndGroupPanel();
	}
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
		_albedoMap = _manager.loadTexture(path.value(), ImageType::ColorSrgb);
		_albedoValueTexture = {};
		_albedoValueTextureView = {};
	}
	else
	{
		_albedoMapPath = std::nullopt;
		_albedoMap = nullptr;
		
		_albedoValueTexture = VKImage::create(
			Engine::getVKContext(),
			vk::Format::eR8G8B8A8Srgb,
			{1, 1},
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_albedoValueTextureView = VKImageView::create(
			Engine::getVKContext(),
			_albedoValueTexture,
			vk::ImageViewType::e2D);
		
		setAlbedoValue(getAlbedoValue()); // updates the texture with the current value
	}
}

const VKPtr<VKImageView>& MaterialAsset::getAlbedoTextureView() const
{
	return _albedoMap != nullptr && _albedoMap->isLoaded() ? _albedoMap->getImageView() : _albedoValueTextureView;
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
		_normalMap = _manager.loadTexture(*path, ImageType::NormalMap);
		_normalValueTexture = {};
		_normalValueTextureView = {};
	}
	else
	{
		_normalMapPath = std::nullopt;
		_normalMap = nullptr;
		
		_normalValueTexture = VKImage::create(
			Engine::getVKContext(),
			vk::Format::eR8G8Unorm,
			{1, 1},
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_normalValueTextureView = VKImageView::create(
			Engine::getVKContext(),
			_normalValueTexture,
			vk::ImageViewType::e2D);
		
		updateImageData(_normalValueTexture, glm::u8vec2{128, 128});
	}
}

const VKPtr<VKImageView>& MaterialAsset::getNormalTextureView() const
{
	return _normalMap != nullptr && _normalMap->isLoaded() ? _normalMap->getImageView() : _normalValueTextureView;
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
		_roughnessMap = _manager.loadTexture(*path, ImageType::Grayscale);
		_roughnessValueTexture = {};
		_roughnessValueTextureView = {};
	}
	else
	{
		_roughnessMapPath = std::nullopt;
		_roughnessMap = nullptr;
		
		_roughnessValueTexture = VKImage::create(
			Engine::getVKContext(),
			vk::Format::eR8Unorm,
			{1, 1},
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_roughnessValueTextureView = VKImageView::create(
			Engine::getVKContext(),
			_roughnessValueTexture,
			vk::ImageViewType::e2D);
		
		setRoughnessValue(getRoughnessValue()); // updates the texture with the current value
	}
}

const VKPtr<VKImageView>& MaterialAsset::getRoughnessTextureView() const
{
	return _roughnessMap != nullptr && _roughnessMap->isLoaded() ? _roughnessMap->getImageView() : _roughnessValueTextureView;
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
		_metalnessMap = _manager.loadTexture(*path, ImageType::Grayscale);
		_metalnessValueTexture = {};
		_metalnessValueTextureView = {};
	}
	else
	{
		_metalnessMapPath = std::nullopt;
		_metalnessMap = nullptr;
		
		_metalnessValueTexture = VKImage::create(
			Engine::getVKContext(),
			vk::Format::eR8Unorm,
			{1, 1},
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_metalnessValueTextureView = VKImageView::create(
			Engine::getVKContext(),
			_metalnessValueTexture,
			vk::ImageViewType::e2D);
		
		setMetalnessValue(getMetalnessValue()); // updates the texture with the current value
	}
}

const VKPtr<VKImageView>& MaterialAsset::getMetalnessTextureView() const
{
	return _metalnessMap != nullptr && _metalnessMap->isLoaded() ? _metalnessMap->getImageView() : _metalnessValueTextureView;
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
		_displacementMap = _manager.loadTexture(*path, ImageType::Grayscale);
		_displacementValueTexture = {};
		_displacementValueTextureView = {};
	}
	else
	{
		_displacementMapPath = std::nullopt;
		_displacementMap = nullptr;
		
		_displacementValueTexture = VKImage::create(
			Engine::getVKContext(),
			vk::Format::eR8Unorm,
			{1, 1},
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_displacementValueTextureView = VKImageView::create(
			Engine::getVKContext(),
			_displacementValueTexture,
			vk::ImageViewType::e2D);
		
		updateImageData<uint8_t>(_displacementValueTexture, 255);
	}
}

const VKPtr<VKImageView>& MaterialAsset::getDisplacementTextureView() const
{
	return _displacementMap != nullptr && _displacementMap->isLoaded() ? _displacementMap->getImageView() : _displacementValueTextureView;
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
		_emissiveMap = _manager.loadTexture(*path, ImageType::Grayscale);
		_emissiveValueTexture = {};
		_emissiveValueTextureView = {};
	}
	else
	{
		_emissiveMapPath = std::nullopt;
		_emissiveMap = nullptr;
		
		_emissiveValueTexture = VKImage::create(
			Engine::getVKContext(),
			vk::Format::eR8Unorm,
			{1, 1},
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_emissiveValueTextureView = VKImageView::create(
			Engine::getVKContext(),
			_emissiveValueTexture,
			vk::ImageViewType::e2D);
		
		setEmissiveValue(getEmissiveValue()); // updates the texture with the current value
	}
}

const VKPtr<VKImageView>& MaterialAsset::getEmissiveTextureView() const
{
	return _emissiveMap != nullptr && _emissiveMap->isLoaded() ? _emissiveMap->getImageView() : _emissiveValueTextureView;
}

const glm::vec3& MaterialAsset::getAlbedoValue() const
{
	return _albedoValue;
}

void MaterialAsset::setAlbedoValue(const glm::vec3& value)
{
	_albedoValue = glm::clamp(value, glm::vec3(0.0f), glm::vec3(1.0f));
	
	if (_albedoValueTexture)
	{
		glm::u8vec4 u8Value = glm::u8vec4(glm::round(_albedoValue * 255.0f), 255);
		updateImageData(_albedoValueTexture, u8Value);
	}
}

const float& MaterialAsset::getRoughnessValue() const
{
	return _roughnessValue;
}

void MaterialAsset::setRoughnessValue(const float& value)
{
	_roughnessValue = glm::clamp(value, 0.0f, 1.0f);

	if (_roughnessValueTexture)
	{
		uint8_t u8Value = glm::round(_roughnessValue * 255.0f);
		updateImageData(_roughnessValueTexture, u8Value);
	}
}

const float& MaterialAsset::getMetalnessValue() const
{
	return _metalnessValue;
}

void MaterialAsset::setMetalnessValue(const float& value)
{
	_metalnessValue = glm::clamp(value, 0.0f, 1.0f);;

	if (_metalnessValueTexture)
	{
		uint8_t u8Value = glm::round(_metalnessValue * 255.0f);
		updateImageData(_metalnessValueTexture, u8Value);
	}
}

const float& MaterialAsset::getEmissiveValue() const
{
	return _emissiveValue;
}

void MaterialAsset::setEmissiveValue(const float& value)
{
	_emissiveValue = glm::clamp(value, 0.0f, 1.0f);;

	if (_emissiveValueTexture)
	{
		uint8_t u8Value = glm::round(_emissiveValue * 255.0f);
		updateImageData(_emissiveValueTexture, u8Value);
	}
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

void MaterialAsset::save() const
{
	nlohmann::ordered_json jsonRoot;
	jsonRoot["version"] = 2;

	{
		nlohmann::ordered_json jsonMap;
		const std::string* path = getAlbedoMapPath();
		if (path != nullptr)
		{
			jsonMap["path"] = *path;
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
		const std::string* path = getNormalMapPath();
		if (path != nullptr)
		{
			jsonMap["path"] = *path;
		}
		else
		{
			jsonMap["path"] = nullptr;
		}

		jsonRoot["normal"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		const std::string* path = getRoughnessMapPath();
		if (path != nullptr)
		{
			jsonMap["path"] = *path;
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
		const std::string* path = getMetalnessMapPath();
		if (path != nullptr)
		{
			jsonMap["path"] = *path;
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
		const std::string* path = getDisplacementMapPath();
		if (path != nullptr)
		{
			jsonMap["path"] = *path;
		}
		else
		{
			jsonMap["path"] = nullptr;
		}

		jsonRoot["displacement"] = jsonMap;
	}

	{
		nlohmann::ordered_json jsonMap;
		const std::string* path = getEmissiveMapPath();
		if (path != nullptr)
		{
			jsonMap["path"] = *path;
		}
		else
		{
			jsonMap["path"] = nullptr;
		}

		const float& value = getEmissiveValue();
		jsonMap["value"] = value;

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
		default:
			throw;
	}
}