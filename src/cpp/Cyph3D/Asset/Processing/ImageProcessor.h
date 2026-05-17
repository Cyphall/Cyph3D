#pragma once

#include "Cyph3D/Asset/Processing/ImageData.h"

#include <filesystem>

class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKComputePipeline;

class ImageProcessor
{
public:
	ImageProcessor();

	ImageData readImageData(std::string_view path, ImageType type, std::string_view cachePath);

private:
	std::shared_ptr<VKDescriptorSetLayout> _descriptorSetLayout;
	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::shared_ptr<VKComputePipeline> _pipeline;

	ImageData processImage(const std::filesystem::path& input, const std::filesystem::path& output, ImageType type);
	ImageData genMipmaps(vk::Format format, glm::uvec2 size, std::span<const std::byte> data, bool isSrgb);
};