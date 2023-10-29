#include "ImageCompressor.h"

#include <vulkan/vulkan_format_traits.hpp>
#include <ispc_texcomp.h>
#include <half.hpp>

static void compressImageBC4(const std::vector<std::byte>& uncompressedImage, const glm::uvec2& uncompressedImageSize, std::vector<std::byte>& compressedImage)
{
	rgba_surface src{
		.ptr = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(uncompressedImage.data())),
		.width = static_cast<int32_t>(uncompressedImageSize.x),
		.height = static_cast<int32_t>(uncompressedImageSize.y),
		.stride = static_cast<int32_t>(uncompressedImageSize.x * sizeof(uint8_t) * 1)
	};
	
	uint8_t* dst = reinterpret_cast<uint8_t*>(compressedImage.data());
	
	CompressBlocksBC4(&src, dst);
}

static void compressImageBC5(const std::vector<std::byte>& uncompressedImage, const glm::uvec2& uncompressedImageSize, std::vector<std::byte>& compressedImage)
{
	rgba_surface src{
		.ptr = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(uncompressedImage.data())),
		.width = static_cast<int32_t>(uncompressedImageSize.x),
		.height = static_cast<int32_t>(uncompressedImageSize.y),
		.stride = static_cast<int32_t>(uncompressedImageSize.x * sizeof(uint8_t) * 2)
	};
	
	uint8_t* dst = reinterpret_cast<uint8_t*>(compressedImage.data());
	
	CompressBlocksBC5(&src, dst);
}

static void compressImageBC6(const std::vector<std::byte>& uncompressedImage, const glm::uvec2& uncompressedImageSize, std::vector<std::byte>& compressedImage)
{
	bc6h_enc_settings settings{};
	GetProfile_bc6h_veryfast(&settings);
	
	rgba_surface src{
		.ptr = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(uncompressedImage.data())),
		.width = static_cast<int32_t>(uncompressedImageSize.x),
		.height = static_cast<int32_t>(uncompressedImageSize.y),
		.stride = static_cast<int32_t>(uncompressedImageSize.x * sizeof(half_float::half) * 4)
	};
	
	uint8_t* dst = reinterpret_cast<uint8_t*>(compressedImage.data());
	
	CompressBlocksBC6H(&src, dst, &settings);
}

static void compressImageBC7(const std::vector<std::byte>& uncompressedImage, const glm::uvec2& uncompressedImageSize, std::vector<std::byte>& compressedImage)
{
	bc7_enc_settings settings{};
	GetProfile_ultrafast(&settings);
	
	rgba_surface src{
		.ptr = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(uncompressedImage.data())),
		.width = static_cast<int32_t>(uncompressedImageSize.x),
		.height = static_cast<int32_t>(uncompressedImageSize.y),
		.stride = static_cast<int32_t>(uncompressedImageSize.x * sizeof(uint8_t) * 4)
	};
	
	uint8_t* dst = reinterpret_cast<uint8_t*>(compressedImage.data());
	
	CompressBlocksBC7(&src, dst, &settings);
}

bool ImageCompressor::tryCompressImage(const std::vector<std::byte>& uncompressedImage, const glm::uvec2& uncompressedImageSize, vk::Format compressedFormat, std::vector<std::byte>& compressedImage)
{
	if (uncompressedImageSize.x % vk::blockExtent(compressedFormat)[0] != 0 ||
		uncompressedImageSize.y % vk::blockExtent(compressedFormat)[1] != 0)
	{
		return false;
	}
	
	glm::uvec2 blockCount(
		uncompressedImageSize.x / vk::blockExtent(compressedFormat)[0],
		uncompressedImageSize.y / vk::blockExtent(compressedFormat)[1]
	);
	
	compressedImage = std::vector<std::byte>(blockCount.x * blockCount.y * vk::blockSize(compressedFormat));
	
	switch (compressedFormat)
	{
		case vk::Format::eBc4UnormBlock:
			compressImageBC4(uncompressedImage, uncompressedImageSize, compressedImage);
			break;
		case vk::Format::eBc5UnormBlock:
			compressImageBC5(uncompressedImage, uncompressedImageSize, compressedImage);
			break;
		case vk::Format::eBc6HUfloatBlock:
			compressImageBC6(uncompressedImage, uncompressedImageSize, compressedImage);
			break;
		case vk::Format::eBc7SrgbBlock:
			compressImageBC7(uncompressedImage, uncompressedImageSize, compressedImage);
			break;
		default:
			return false;
	}
	
	return true;
}