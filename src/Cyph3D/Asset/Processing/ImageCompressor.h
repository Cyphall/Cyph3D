#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

class ImageCompressor
{
public:
	static void init();
	static bool tryCompressImage(const std::vector<std::byte>& uncompressedImage, const glm::uvec2& uncompressedImageSize, vk::Format compressedFormat, std::vector<std::byte>& compressedImage);
};