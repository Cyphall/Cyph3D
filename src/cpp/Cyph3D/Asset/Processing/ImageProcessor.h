#pragma once

#include <Cyph3D/Asset/Processing/ImageData.h>

#include <CyphGPU/fwd.hpp>
#include <filesystem>

namespace c3d
{
class ImageProcessor
{
public:
	ImageData readImageData(std::string_view path, ImageType type, std::string_view cachePath);

private:
	ImageData processImage(const std::filesystem::path& input, const std::filesystem::path& output, ImageType type);
	ImageData genMipmaps(vk::Format format, glm::uvec2 extent, std::span<const std::byte> data);
};
}
