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
	VKPtr<VKDescriptorSetLayout> _cubemapDescriptorSetLayout;
	VKPtr<VKPipelineLayout> _cubemapPipelineLayout;
	VKPtr<VKComputePipeline> _cubemapPipeline;
	VKPtr<VKSampler> _cubemapSampler;

	VKPtr<VKDescriptorSetLayout> _mipmapDescriptorSetLayout;
	VKPtr<VKPipelineLayout> _mipmapPipelineLayout;
	VKPtr<VKComputePipeline> _mipmapPipeline;

	EquirectangularSkyboxData processEquirectangularSkybox(AssetManagerWorkerData& workerData, const std::filesystem::path& input, const std::filesystem::path& output);
	EquirectangularSkyboxData genCubemapAndMipmaps(AssetManagerWorkerData& workerData, vk::Format format, glm::uvec2 size, std::span<const std::byte> data, bool isSrgb);
	VKPtr<VKImage> generateCubemap(AssetManagerWorkerData& workerData, vk::Format format, const VKPtr<VKImage>& equirectangularTexture);
	void generateMipmaps(AssetManagerWorkerData& workerData, const VKPtr<VKImage>& cubemapTexture, bool isSrgb);
};