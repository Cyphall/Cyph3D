#include "ImageCompressor.h"

#include <bc7enc.h>
#include <rgbcx.h>
#include <vulkan/vulkan_format_traits.hpp>

static void compressImageBC4(const std::vector<std::byte>& uncompressedImage, const glm::uvec2& uncompressedImageSize, const glm::uvec2& compressedBlockCount, std::vector<std::byte>& compressedImage)
{
	glm::uvec2 inputPixelOffset{
		1,
		uncompressedImageSize.x * 1
	};
	
	glm::uvec2 inputBlockOffset = inputPixelOffset * glm::uvec2(4, 4);
	
	std::array<uint8_t, 16> blockInput{};
	std::array<uint8_t, 8> blockOutput{};
	
	std::byte* outputPtr = compressedImage.data();
	for (int blockY = 0; blockY < compressedBlockCount.y; blockY++)
	{
		for (int blockX = 0; blockX < compressedBlockCount.x; blockX++)
		{
			size_t baseInputOffset = (blockX * inputBlockOffset.x) + (blockY * inputBlockOffset.y);
			std::memcpy(blockInput.data() + 0,  uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 0), 4);
			std::memcpy(blockInput.data() + 4,  uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 1), 4);
			std::memcpy(blockInput.data() + 8,  uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 2), 4);
			std::memcpy(blockInput.data() + 12, uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 3), 4);
			
			rgbcx::encode_bc4(blockOutput.data(), blockInput.data(), 1);
			
			std::memcpy(outputPtr, blockOutput.data(), 8);
			outputPtr += 8;
		}
	}
}

static void compressImageBC5(const std::vector<std::byte>& uncompressedImage, const glm::uvec2& uncompressedImageSize, const glm::uvec2& compressedBlockCount, std::vector<std::byte>& compressedImage)
{
	glm::uvec2 inputPixelOffset{
		2,
		uncompressedImageSize.x * 2
	};
	
	glm::uvec2 inputBlockOffset = inputPixelOffset * glm::uvec2(4, 4);
	
	std::array<uint8_t, 32> blockInput{};
	std::array<uint8_t, 16> blockOutput{};
	
	std::byte* outputPtr = compressedImage.data();
	for (int blockY = 0; blockY < compressedBlockCount.y; blockY++)
	{
		for (int blockX = 0; blockX < compressedBlockCount.x; blockX++)
		{
			size_t baseInputOffset = (blockX * inputBlockOffset.x) + (blockY * inputBlockOffset.y);
			std::memcpy(blockInput.data() + 0,  uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 0), 8);
			std::memcpy(blockInput.data() + 8,  uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 1), 8);
			std::memcpy(blockInput.data() + 16, uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 2), 8);
			std::memcpy(blockInput.data() + 24, uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 3), 8);
			
			rgbcx::encode_bc5(blockOutput.data(), blockInput.data(), 0, 1, 2);
			
			std::memcpy(outputPtr, blockOutput.data(), 16);
			outputPtr += 16;
		}
	}
}

static void compressImageBC7(const std::vector<std::byte>& uncompressedImage, const glm::uvec2& uncompressedImageSize, const glm::uvec2& compressedBlockCount, std::vector<std::byte>& compressedImage)
{
	glm::uvec2 inputPixelOffset{
		4,
		uncompressedImageSize.x * 4
	};
	
	glm::uvec2 inputBlockOffset = inputPixelOffset * glm::uvec2(4, 4);
	
	bc7enc_compress_block_params params{};
	bc7enc_compress_block_params_init(&params);
	params.m_max_partitions = 0;
	params.m_uber_level = 0;
	params.m_mode17_partition_estimation_filterbank = true;
	params.m_try_least_squares = false;
	
	std::array<uint8_t, 64> blockInput{};
	std::array<uint8_t, 16> blockOutput{};
	
	std::byte* outputPtr = compressedImage.data();
	for (int blockY = 0; blockY < compressedBlockCount.y; blockY++)
	{
		for (int blockX = 0; blockX < compressedBlockCount.x; blockX++)
		{
			size_t baseInputOffset = (blockX * inputBlockOffset.x) + (blockY * inputBlockOffset.y);
			std::memcpy(blockInput.data() + 0,  uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 0), 16);
			std::memcpy(blockInput.data() + 16, uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 1), 16);
			std::memcpy(blockInput.data() + 32, uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 2), 16);
			std::memcpy(blockInput.data() + 48, uncompressedImage.data() + baseInputOffset + (inputPixelOffset.y * 3), 16);
			
			bc7enc_compress_block(blockOutput.data(), blockInput.data(), &params);
			
			std::memcpy(outputPtr, blockOutput.data(), 16);
			outputPtr += 16;
		}
	}
}

void ImageCompressor::init()
{
	bc7enc_compress_block_init();
	rgbcx::init();
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
		case vk::Format::eBc4SnormBlock:
			compressImageBC4(uncompressedImage, uncompressedImageSize, blockCount, compressedImage);
			break;
		case vk::Format::eBc5UnormBlock:
		case vk::Format::eBc5SnormBlock:
			compressImageBC5(uncompressedImage, uncompressedImageSize, blockCount, compressedImage);
			break;
		case vk::Format::eBc7UnormBlock:
		case vk::Format::eBc7SrgbBlock:
			compressImageBC7(uncompressedImage, uncompressedImageSize, blockCount, compressedImage);
			break;
		default:
			return false;
	}
	
	return true;
}