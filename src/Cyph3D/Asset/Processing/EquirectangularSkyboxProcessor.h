#pragma once

#include "Cyph3D/Asset/AssetManagerWorkerData.h"
#include "Cyph3D/Asset/Processing/EquirectangularSkyboxData.h"

#include <filesystem>

class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKComputePipeline;
class VKSampler;
class VKImage;

class EquirectangularSkyboxProcessor
{
public:
	EquirectangularSkyboxProcessor();

	EquirectangularSkyboxData readEquirectangularSkyboxData(AssetManagerWorkerData& workerData, std::string_view path, std::string_view cachePath);

private:
	std::shared_ptr<VKDescriptorSetLayout> _cubemapDescriptorSetLayout;
	std::shared_ptr<VKPipelineLayout> _cubemapPipelineLayout;
	std::shared_ptr<VKComputePipeline> _cubemapPipeline;
	std::shared_ptr<VKSampler> _cubemapSampler;

	std::shared_ptr<VKDescriptorSetLayout> _mipmapDescriptorSetLayout;
	std::shared_ptr<VKPipelineLayout> _mipmapPipelineLayout;
	std::shared_ptr<VKComputePipeline> _mipmapPipeline;

	EquirectangularSkyboxData processEquirectangularSkybox(AssetManagerWorkerData& workerData, const std::filesystem::path& input, const std::filesystem::path& output);
	EquirectangularSkyboxData genCubemapAndMipmaps(AssetManagerWorkerData& workerData, vk::Format format, glm::uvec2 size, std::span<const std::byte> data, bool isSrgb);
	std::shared_ptr<VKImage> generateCubemap(AssetManagerWorkerData& workerData, vk::Format format, const std::shared_ptr<VKImage>& equirectangularTexture);
	void generateMipmaps(AssetManagerWorkerData& workerData, const std::shared_ptr<VKImage>& cubemapTexture, bool isSrgb);
};