#pragma once

#include "Cyph3D/Asset/AssetManagerWorkerData.h"
#include "Cyph3D/Asset/Processing/ImageData.h"

#include <filesystem>
#include <string_view>

class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKComputePipeline;

class ImageProcessor
{
public:
	ImageProcessor();

	ImageData readImageData(AssetManagerWorkerData& workerData, std::string_view path, ImageType type, std::string_view cachePath);

private:
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKComputePipeline> _pipeline;

	ImageData processImage(AssetManagerWorkerData& workerData, const std::filesystem::path& input, const std::filesystem::path& output, ImageType type);
	ImageData genMipmaps(AssetManagerWorkerData& workerData, vk::Format format, glm::uvec2 size, std::span<const std::byte> data, bool isSrgb);
};