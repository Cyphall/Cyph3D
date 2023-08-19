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

static void createVKImageAndView(vk::Format format, VKPtr<VKImage>& image, VKPtr<VKImageView>& imageView)
{
	VKImageInfo imageInfo(
		format,
		{1, 1},
		1,
		1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	image = VKImage::create(Engine::getVKContext(), imageInfo);
	
	VKImageViewInfo imageViewInfo(
		image,
		vk::ImageViewType::e2D);
	
	imageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
}

template<typename T>
static void uploadImageData(const VKPtr<VKImage>& image, const T& value)
{
	VKBufferInfo bufferInfo(1, vk::BufferUsageFlagBits::eTransferSrc);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	
	VKPtr<VKBuffer<T>> stagingBuffer = VKBuffer<T>::create(Engine::getVKContext(), bufferInfo);
	
	std::memcpy(stagingBuffer->getHostPointer(), &value, sizeof(T));
	
	Engine::getVKContext().executeImmediate(
		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
		{
			commandBuffer->imageMemoryBarrier(
				image,
				0,
				0,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eTransferDstOptimal);
			
			commandBuffer->copyBufferToImage(stagingBuffer, 0, image, 0, 0);
			
			commandBuffer->imageMemoryBarrier(
				image,
				0,
				0,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal);
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
	return ((_albedoMap != nullptr && _albedoMap->isLoaded()) || _albedoValueTextureBindlessIndex) &&
	       ((_normalMap != nullptr && _normalMap->isLoaded()) || _normalValueTextureBindlessIndex) &&
	       ((_roughnessMap != nullptr && _roughnessMap->isLoaded()) || _roughnessValueTextureBindlessIndex) &&
	       ((_metalnessMap != nullptr && _metalnessMap->isLoaded()) || _metalnessValueTextureBindlessIndex) &&
	       ((_displacementMap != nullptr && _displacementMap->isLoaded()) || _displacementValueTextureBindlessIndex) &&
	       ((_emissiveMap != nullptr && _emissiveMap->isLoaded()) || _emissiveValueTextureBindlessIndex);
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
		
		float scale = getEmissiveScale();
		if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.0f, FLT_MAX, "%.2f"))
		{
			setEmissiveScale(scale);
		}

		ImGuiHelper::EndGroupPanel();
	}
}

const std::string* MaterialAsset::getAlbedoMapPath() const
{
	return _albedoMap ? &_albedoMap->getSignature().path : nullptr;
}

void MaterialAsset::setAlbedoMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_albedoValueTexture = {};
		_albedoValueTextureView = {};
		
		if (_albedoValueTextureBindlessIndex)
		{
			_manager.getBindlessTextureManager().releaseIndex(*_albedoValueTextureBindlessIndex);
			_albedoValueTextureBindlessIndex = std::nullopt;
		}
		
		_albedoMap = _manager.loadTexture(path.value(), ImageType::ColorSrgb);
		_albedoMapChangedConnection = _albedoMap->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_albedoMap = nullptr;
		_albedoMapChangedConnection = {};
		
		if (!_albedoValueTextureBindlessIndex)
		{
			createVKImageAndView(vk::Format::eR8G8B8A8Srgb, _albedoValueTexture, _albedoValueTextureView);
			uploadAlbedoValue(_albedoValue);
			
			_albedoValueTextureBindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
			_manager.getBindlessTextureManager().setTexture(*_albedoValueTextureBindlessIndex, _albedoValueTextureView, _manager.getTextureSampler());
		}
	}
	
	_changed();
}

const uint32_t& MaterialAsset::getAlbedoTextureBindlessIndex() const
{
	return _albedoMap != nullptr && _albedoMap->isLoaded() ? _albedoMap->getBindlessIndex() : *_albedoValueTextureBindlessIndex;
}

const std::string* MaterialAsset::getNormalMapPath() const
{
	return _normalMap ? &_normalMap->getSignature().path : nullptr;
}

void MaterialAsset::setNormalMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_normalValueTexture = {};
		_normalValueTextureView = {};
		
		if (_normalValueTextureBindlessIndex)
		{
			_manager.getBindlessTextureManager().releaseIndex(*_normalValueTextureBindlessIndex);
			_normalValueTextureBindlessIndex = std::nullopt;
		}
		
		_normalMap = _manager.loadTexture(*path, ImageType::NormalMap);
		_normalMapChangedConnection = _normalMap->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_normalMap = nullptr;
		_normalMapChangedConnection = {};
		
		if (!_normalValueTextureBindlessIndex)
		{
			createVKImageAndView(vk::Format::eR8G8Unorm, _normalValueTexture, _normalValueTextureView);
			uploadNormalValue({0.5f, 0.5f, 1.0f});
			
			_normalValueTextureBindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
			_manager.getBindlessTextureManager().setTexture(*_normalValueTextureBindlessIndex, _normalValueTextureView, _manager.getTextureSampler());
		}
	}
	
	_changed();
}

const uint32_t& MaterialAsset::getNormalTextureBindlessIndex() const
{
	return _normalMap != nullptr && _normalMap->isLoaded() ? _normalMap->getBindlessIndex() : *_normalValueTextureBindlessIndex;
}

const std::string* MaterialAsset::getRoughnessMapPath() const
{
	return _roughnessMap ? &_roughnessMap->getSignature().path : nullptr;
}

void MaterialAsset::setRoughnessMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_roughnessValueTexture = {};
		_roughnessValueTextureView = {};
		
		if (_roughnessValueTextureBindlessIndex)
		{
			_manager.getBindlessTextureManager().releaseIndex(*_roughnessValueTextureBindlessIndex);
			_roughnessValueTextureBindlessIndex = std::nullopt;
		}
		
		_roughnessMap = _manager.loadTexture(*path, ImageType::Grayscale);
		_roughnessMapChangedConnection = _roughnessMap->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_roughnessMap = nullptr;
		_roughnessMapChangedConnection = {};
		
		if (!_roughnessValueTextureBindlessIndex)
		{
			createVKImageAndView(vk::Format::eR8Unorm, _roughnessValueTexture, _roughnessValueTextureView);
			uploadRoughnessValue(_roughnessValue);
			
			_roughnessValueTextureBindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
			_manager.getBindlessTextureManager().setTexture(*_roughnessValueTextureBindlessIndex, _roughnessValueTextureView, _manager.getTextureSampler());
		}
	}
	
	_changed();
}

const uint32_t& MaterialAsset::getRoughnessTextureBindlessIndex() const
{
	return _roughnessMap != nullptr && _roughnessMap->isLoaded() ? _roughnessMap->getBindlessIndex() : *_roughnessValueTextureBindlessIndex;
}

const std::string* MaterialAsset::getMetalnessMapPath() const
{
	return _metalnessMap ? &_metalnessMap->getSignature().path : nullptr;
}

void MaterialAsset::setMetalnessMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_metalnessValueTexture = {};
		_metalnessValueTextureView = {};
		
		if (_metalnessValueTextureBindlessIndex)
		{
			_manager.getBindlessTextureManager().releaseIndex(*_metalnessValueTextureBindlessIndex);
			_metalnessValueTextureBindlessIndex = std::nullopt;
		}
		
		_metalnessMap = _manager.loadTexture(*path, ImageType::Grayscale);
		_metalnessMapChangedConnection = _metalnessMap->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_metalnessMap = nullptr;
		_metalnessMapChangedConnection = {};
		
		if (!_metalnessValueTextureBindlessIndex)
		{
			createVKImageAndView(vk::Format::eR8Unorm, _metalnessValueTexture, _metalnessValueTextureView);
			uploadMetalnessValue(_metalnessValue);
			
			_metalnessValueTextureBindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
			_manager.getBindlessTextureManager().setTexture(*_metalnessValueTextureBindlessIndex, _metalnessValueTextureView, _manager.getTextureSampler());
		}
	}
	
	_changed();
}

const uint32_t& MaterialAsset::getMetalnessTextureBindlessIndex() const
{
	return _metalnessMap != nullptr && _metalnessMap->isLoaded() ? _metalnessMap->getBindlessIndex() : *_metalnessValueTextureBindlessIndex;
}

const std::string* MaterialAsset::getDisplacementMapPath() const
{
	return _displacementMap ? &_displacementMap->getSignature().path : nullptr;
}

void MaterialAsset::setDisplacementMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_displacementValueTexture = {};
		_displacementValueTextureView = {};
		
		if (_displacementValueTextureBindlessIndex)
		{
			_manager.getBindlessTextureManager().releaseIndex(*_displacementValueTextureBindlessIndex);
			_displacementValueTextureBindlessIndex = std::nullopt;
		}
		
		_displacementMap = _manager.loadTexture(*path, ImageType::Grayscale);
		_displacementMapChangedConnection = _displacementMap->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_displacementMap = nullptr;
		_displacementMapChangedConnection = {};
		
		if (!_displacementValueTextureBindlessIndex)
		{
			createVKImageAndView(vk::Format::eR8Unorm, _displacementValueTexture, _displacementValueTextureView);
			uploadDisplacementValue(1.0f);
			
			_displacementValueTextureBindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
			_manager.getBindlessTextureManager().setTexture(*_displacementValueTextureBindlessIndex, _displacementValueTextureView, _manager.getTextureSampler());
		}
	}
	
	_changed();
}

const uint32_t& MaterialAsset::getDisplacementTextureBindlessIndex() const
{
	return _displacementMap != nullptr && _displacementMap->isLoaded() ? _displacementMap->getBindlessIndex() : *_displacementValueTextureBindlessIndex;
}

const std::string* MaterialAsset::getEmissiveMapPath() const
{
	return _emissiveMap ? &_emissiveMap->getSignature().path : nullptr;
}

void MaterialAsset::setEmissiveMapPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_emissiveValueTexture = {};
		_emissiveValueTextureView = {};
		
		if (_emissiveValueTextureBindlessIndex)
		{
			_manager.getBindlessTextureManager().releaseIndex(*_emissiveValueTextureBindlessIndex);
			_emissiveValueTextureBindlessIndex = std::nullopt;
		}
		
		_emissiveMap = _manager.loadTexture(*path, ImageType::Grayscale);
		_emissiveMapChangedConnection = _emissiveMap->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_emissiveMap = nullptr;
		_emissiveMapChangedConnection = {};
		
		if (!_emissiveValueTextureBindlessIndex)
		{
			createVKImageAndView(vk::Format::eR8Unorm, _emissiveValueTexture, _emissiveValueTextureView);
			uploadEmissiveValue(1.0f);
			
			_emissiveValueTextureBindlessIndex = _manager.getBindlessTextureManager().acquireIndex();
			_manager.getBindlessTextureManager().setTexture(*_emissiveValueTextureBindlessIndex, _emissiveValueTextureView, _manager.getTextureSampler());
		}
	}
	
	_changed();
}

const uint32_t& MaterialAsset::getEmissiveTextureBindlessIndex() const
{
	return _emissiveMap != nullptr && _emissiveMap->isLoaded() ? _emissiveMap->getBindlessIndex() : *_emissiveValueTextureBindlessIndex;
}

const glm::vec3& MaterialAsset::getAlbedoValue() const
{
	return _albedoValue;
}

void MaterialAsset::setAlbedoValue(const glm::vec3& value)
{
	_albedoValue = glm::clamp(value, glm::vec3(0.0f), glm::vec3(1.0f));
	
	if (_albedoValueTextureBindlessIndex)
	{
		uploadAlbedoValue(_albedoValue);
	}
	
	_changed();
}

const float& MaterialAsset::getRoughnessValue() const
{
	return _roughnessValue;
}

void MaterialAsset::setRoughnessValue(const float& value)
{
	_roughnessValue = glm::clamp(value, 0.0f, 1.0f);

	if (_roughnessValueTextureBindlessIndex)
	{
		uploadRoughnessValue(_roughnessValue);
	}
	
	_changed();
}

const float& MaterialAsset::getMetalnessValue() const
{
	return _metalnessValue;
}

void MaterialAsset::setMetalnessValue(const float& value)
{
	_metalnessValue = glm::clamp(value, 0.0f, 1.0f);;

	if (_metalnessValueTextureBindlessIndex)
	{
		uploadMetalnessValue(_metalnessValue);
	}
	
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
			
			setEmissiveScale(1.0f);
		}
		else
		{
			setEmissiveMapPath(std::nullopt);
			
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
		
		const nlohmann::ordered_json& jsonValue = jsonMap.at("scale");
		setEmissiveScale(jsonValue.get<float>());
	}
}

void MaterialAsset::save() const
{
	nlohmann::ordered_json jsonRoot;
	jsonRoot["version"] = 3;

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
		default:
			throw;
	}
}

void MaterialAsset::uploadAlbedoValue(const glm::vec3& albedo)
{
	glm::u8vec4 uploadValue = glm::u8vec4(glm::round(albedo * 255.0f), 255);
	uploadImageData(_albedoValueTexture, uploadValue);
}

void MaterialAsset::uploadNormalValue(const glm::vec3& normal)
{
	glm::u8vec2 uploadValue = glm::round(normal * 255.0f);
	uploadImageData(_normalValueTexture, uploadValue);
}

void MaterialAsset::uploadRoughnessValue(const float& roughness)
{
	uint8_t uploadValue = glm::round(roughness * 255.0f);
	uploadImageData(_roughnessValueTexture, uploadValue);
}

void MaterialAsset::uploadMetalnessValue(const float& metalness)
{
	uint8_t uploadValue = glm::round(metalness * 255.0f);
	uploadImageData(_metalnessValueTexture, uploadValue);
}

void MaterialAsset::uploadDisplacementValue(const float& displacement)
{
	uint8_t uploadValue = glm::round(displacement * 255.0f);
	uploadImageData(_displacementValueTexture, uploadValue);
}

void MaterialAsset::uploadEmissiveValue(const float& emissive)
{
	uint8_t uploadValue = glm::round(emissive * 255.0f);
	uploadImageData(_emissiveValueTexture, uploadValue);
}