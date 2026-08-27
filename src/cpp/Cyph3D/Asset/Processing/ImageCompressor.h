#pragma once

#include <glm/glm.hpp>
#include <span>
#include <vulkan/vulkan.hpp>

namespace c3d
{
class ImageCompressor
{
public:
	static void compressImage(std::span<const std::byte> uncompressedImage, const glm::uvec2& uncompressedExtent, vk::Format compressedFormat, std::span<std::byte> compressedImage);
};
}
