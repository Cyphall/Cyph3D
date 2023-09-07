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
	return ((_albedoTexture != nullptr && _albedoTexture->isLoaded()) || _albedoValueTextureBindlessIndex) &&
	       ((_normalTexture != nullptr && _normalTexture->isLoaded()) || _normalValueTextureBindlessIndex) &&
	       ((_roughnessTexture != nullptr && _roughnessTexture->isLoaded()) || _roughnessValueTextureBindlessIndex) &&
	       ((_metalnessTexture != nullptr && _metalnessTexture->isLoaded()) || _metalnessValueTextureBindlessIndex) &&
	       ((_displacementTexture != nullptr && _displacementTexture->isLoaded()) || _displacementValueTextureBindlessIndex) &&
	       ((_emissiveTexture != nullptr && _emissiveTexture->isLoaded()) || _emissiveValueTextureBindlessIndex);
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
		_albedoValueTexture = {};
		_albedoValueTextureView = {};
		
		if (_albedoValueTextureBindlessIndex)
		{
			_manager.getBindlessTextureManager().releaseIndex(*_albedoValueTextureBindlessIndex);
			_albedoValueTextureBindlessIndex = std::nullopt;
		}
		
		_albedoTexture = _manager.loadTexture(path.value(), ImageType::ColorSrgb);
		_albedoTextureChangedConnection = _albedoTexture->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_albedoTexture = nullptr;
		_albedoTextureChangedConnection = {};
		
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
	return _albedoTexture != nullptr && _albedoTexture->isLoaded() ? _albedoTexture->getBindlessIndex() : *_albedoValueTextureBindlessIndex;
}

void MaterialAsset::setNormalTexture(std::optional<std::string_view> path)
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
		
		_normalTexture = _manager.loadTexture(*path, ImageType::NormalMap);
		_normalTextureChangedConnection = _normalTexture->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_normalTexture = nullptr;
		_normalTextureChangedConnection = {};
		
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
	return _normalTexture != nullptr && _normalTexture->isLoaded() ? _normalTexture->getBindlessIndex() : *_normalValueTextureBindlessIndex;
}

void MaterialAsset::setRoughnessTexture(std::optional<std::string_view> path)
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
		
		_roughnessTexture = _manager.loadTexture(*path, ImageType::Grayscale);
		_roughnessTextureChangedConnection = _roughnessTexture->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_roughnessTexture = nullptr;
		_roughnessTextureChangedConnection = {};
		
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
	return _roughnessTexture != nullptr && _roughnessTexture->isLoaded() ? _roughnessTexture->getBindlessIndex() : *_roughnessValueTextureBindlessIndex;
}

void MaterialAsset::setMetalnessTexture(std::optional<std::string_view> path)
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
		
		_metalnessTexture = _manager.loadTexture(*path, ImageType::Grayscale);
		_metalnessTextureChangedConnection = _metalnessTexture->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_metalnessTexture = nullptr;
		_metalnessTextureChangedConnection = {};
		
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
	return _metalnessTexture != nullptr && _metalnessTexture->isLoaded() ? _metalnessTexture->getBindlessIndex() : *_metalnessValueTextureBindlessIndex;
}

void MaterialAsset::setDisplacementTexture(std::optional<std::string_view> path)
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
		
		_displacementTexture = _manager.loadTexture(*path, ImageType::Grayscale);
		_displacementTextureChangedConnection = _displacementTexture->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_displacementTexture = nullptr;
		_displacementTextureChangedConnection = {};
		
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
	return _displacementTexture != nullptr && _displacementTexture->isLoaded() ? _displacementTexture->getBindlessIndex() : *_displacementValueTextureBindlessIndex;
}

void MaterialAsset::setEmissiveTexture(std::optional<std::string_view> path)
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
		
		_emissiveTexture = _manager.loadTexture(*path, ImageType::Grayscale);
		_emissiveTextureChangedConnection = _emissiveTexture->getChangedSignal().connect([this](){
			_changed();
		});
	}
	else
	{
		_emissiveTexture = nullptr;
		_emissiveTextureChangedConnection = {};
		
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
	return _emissiveTexture != nullptr && _emissiveTexture->isLoaded() ? _emissiveTexture->getBindlessIndex() : *_emissiveValueTextureBindlessIndex;
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
			jsonValue.at(2).get<float>()
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
			               jsonValue.at(2).get<float>()
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
	jsonRoot["version"] = 3;

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