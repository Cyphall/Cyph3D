#include "ImageProcessor.h"

#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/StbImage.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Engine.h"

#include <filesystem>
#include <array>
#include <magic_enum.hpp>
#include <vulkan/vulkan_format_traits.hpp>
#include <bc7enc.h>
#include <rgbcx.h>

static void writeProcessedImage(const std::filesystem::path& path, const ImageData& imageData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file(path, std::ios::out | std::ios::binary);

	uint8_t version = 3;
	FileHelper::write(file, &version);
	
	FileHelper::write(file, &imageData.format);

	FileHelper::write(file, &imageData.size);
	
	uint32_t levels = imageData.levels.size();
	FileHelper::write(file, &levels);
	
	for (uint32_t i = 0; i < levels; i++)
	{
		FileHelper::write(file, imageData.levels[i].data);
	}
}

static bool readProcessedImage(const std::filesystem::path& path, ImageData& imageData)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);

	uint8_t version;
	FileHelper::read(file, &version);

	if (version != 3)
	{
		return false;
	}
	
	FileHelper::read(file, &imageData.format);
	
	FileHelper::read(file, &imageData.size);
	
	uint32_t levels = imageData.levels.size();
	FileHelper::read(file, &levels);
	imageData.levels.resize(levels);
	
	for (uint32_t i = 0; i < levels; i++)
	{
		FileHelper::read(file, imageData.levels[i].data);
	}

	return true;
}

static std::vector<std::byte> convertRgbToRg(std::span<const std::byte> input, int bytesPerChannel)
{
	if (input.size() % 3 != 0)
	{
		throw;
	}
	
	std::vector<std::byte> output((input.size() / 3) * 2);
	for (size_t i = 0; i < output.size(); i++)
	{
		output[i] = input[i + (i / 2 / bytesPerChannel) * bytesPerChannel];
	}
	
	return output;
}

static ImageData genMipmaps(vk::Format format, glm::uvec2 size, std::span<const std::byte> data)
{
	VKPtr<VKImage> texture = VKImage::create(
		Engine::getVKContext(),
		format,
		size,
		1,
		VKImage::calcMaxMipLevels(size),
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc,
		vk::ImageAspectFlagBits::eColor,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(
		Engine::getVKContext(),
		texture->getLayerByteSize(),
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);
	
	std::byte* ptr = stagingBuffer->map();
	std::copy(data.data(), data.data() + texture->getLevelByteSize(0), ptr);
	stagingBuffer->unmap();
	
	Engine::getVKContext().executeImmediate(
		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
		{
			commandBuffer->imageMemoryBarrier(
				texture,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eTransferDstOptimal,
				0,
				0);
			
			commandBuffer->copyBufferToImage(stagingBuffer, 0, texture, 0, 0);
			
			commandBuffer->imageMemoryBarrier(
				texture,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eBlit,
				vk::AccessFlagBits2::eTransferRead,
				vk::ImageLayout::eTransferSrcOptimal,
				0,
				0);
			
			vk::DeviceSize bufferOffset = texture->getLevelByteSize(0);
			for (uint32_t i = 1; i < texture->getLevels(); i++)
			{
				commandBuffer->imageMemoryBarrier(
					texture,
					vk::PipelineStageFlagBits2::eNone,
					vk::AccessFlagBits2::eNone,
					vk::PipelineStageFlagBits2::eBlit,
					vk::AccessFlagBits2::eTransferWrite,
					vk::ImageLayout::eTransferDstOptimal,
					0,
					i);
				
				commandBuffer->blitImage(texture, 0, i-1, texture, 0, i);
				
				commandBuffer->imageMemoryBarrier(
					texture,
					vk::PipelineStageFlagBits2::eBlit,
					vk::AccessFlagBits2::eTransferWrite,
					vk::PipelineStageFlagBits2::eCopy | vk::PipelineStageFlagBits2::eBlit,
					vk::AccessFlagBits2::eTransferRead,
					vk::ImageLayout::eTransferSrcOptimal,
					0,
					i);
				
				commandBuffer->copyImageToBuffer(texture, 0, i, stagingBuffer, bufferOffset);
				bufferOffset += texture->getLevelByteSize(i);
			}
		});
	
	ImageData imageData;
	imageData.format = format;
	imageData.size = size;
	imageData.levels.resize(texture->getLevels());
	
	ptr = stagingBuffer->map();
	for (uint32_t i = 0; i < imageData.levels.size(); i++)
	{
		imageData.levels[i].data.resize(texture->getLevelByteSize(i));
		
		std::memcpy(imageData.levels[i].data.data(), ptr, imageData.levels[i].data.size());
		
		ptr += imageData.levels[i].data.size();
	}
	stagingBuffer->unmap();
	
	return imageData;
}

static void compressLevelBC4(const ImageLevel& uncompressedLevel, ImageLevel& compressedLevel, glm::uvec2 inputSize, glm::uvec2 outputSize)
{
	glm::uvec2 inputPixelOffset{
		1,
		inputSize.x * 1
	};
	
	glm::uvec2 inputBlockOffset = inputPixelOffset * glm::uvec2(4, 4);
	
	rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal);
	
	std::array<uint8_t, 16> blockInput{};
	std::array<uint8_t, 8> blockOutput{};
	
	std::byte* outputPtr = compressedLevel.data.data();
	for (int blockY = 0; blockY < outputSize.y; blockY++)
	{
		for (int blockX = 0; blockX < outputSize.y; blockX++)
		{
			size_t baseInputOffset = (blockX * inputBlockOffset.x) + (blockY * inputBlockOffset.y);
			std::memcpy(blockInput.data() + 0, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 0), 4);
			std::memcpy(blockInput.data() + 4, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 1), 4);
			std::memcpy(blockInput.data() + 8, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 2), 4);
			std::memcpy(blockInput.data() + 12, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 3), 4);
			
			rgbcx::encode_bc4(blockOutput.data(), blockInput.data(), 1);
			
			std::memcpy(outputPtr, blockOutput.data(), 8);
			outputPtr += 8;
		}
	}
}

static void compressLevelBC5(const ImageLevel& uncompressedLevel, ImageLevel& compressedLevel, glm::uvec2 inputSize, glm::uvec2 outputSize)
{
	glm::uvec2 inputPixelOffset{
		2,
		inputSize.x * 2
	};
	
	glm::uvec2 inputBlockOffset = inputPixelOffset * glm::uvec2(4, 4);
	
	rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal);
	
	std::array<uint8_t, 32> blockInput{};
	std::array<uint8_t, 16> blockOutput{};
	
	std::byte* outputPtr = compressedLevel.data.data();
	for (int blockY = 0; blockY < outputSize.y; blockY++)
	{
		for (int blockX = 0; blockX < outputSize.y; blockX++)
		{
			size_t baseInputOffset = (blockX * inputBlockOffset.x) + (blockY * inputBlockOffset.y);
			std::memcpy(blockInput.data() + 0, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 0), 8);
			std::memcpy(blockInput.data() + 8, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 1), 8);
			std::memcpy(blockInput.data() + 16, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 2), 8);
			std::memcpy(blockInput.data() + 24, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 3), 8);
			
			rgbcx::encode_bc5(blockOutput.data(), blockInput.data(), 0, 1, 2);
			
			std::memcpy(outputPtr, blockOutput.data(), 16);
			outputPtr += 16;
		}
	}
}

static void compressLevelBC7(const ImageLevel& uncompressedLevel, ImageLevel& compressedLevel, glm::uvec2 inputSize, glm::uvec2 outputSize)
{
	glm::uvec2 inputPixelOffset{
		4,
		inputSize.x * 4
	};
	
	glm::uvec2 inputBlockOffset = inputPixelOffset * glm::uvec2(4, 4);
	
	bc7enc_compress_block_init();
	
	bc7enc_compress_block_params params{};
	bc7enc_compress_block_params_init(&params);
	params.m_max_partitions = 0;
	params.m_uber_level = 0;
	params.m_mode17_partition_estimation_filterbank = true;
	params.m_try_least_squares = false;
	
	std::array<uint8_t, 64> blockInput{};
	std::array<uint8_t, 16> blockOutput{};
	
	std::byte* outputPtr = compressedLevel.data.data();
	for (int blockY = 0; blockY < outputSize.y; blockY++)
	{
		for (int blockX = 0; blockX < outputSize.y; blockX++)
		{
			size_t baseInputOffset = (blockX * inputBlockOffset.x) + (blockY * inputBlockOffset.y);
			std::memcpy(blockInput.data() + 0, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 0), 16);
			std::memcpy(blockInput.data() + 16, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 1), 16);
			std::memcpy(blockInput.data() + 32, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 2), 16);
			std::memcpy(blockInput.data() + 48, uncompressedLevel.data.data() + baseInputOffset + (inputPixelOffset.y * 3), 16);
			
			bc7enc_compress_block(blockOutput.data(), blockInput.data(), &params);
			
			std::memcpy(outputPtr, blockOutput.data(), 16);
			outputPtr += 16;
		}
	}
}

ImageData compressTexture(const ImageData& mipmappedImageData, vk::Format requestedFormat)
{
	ImageData compressedImageData;
	compressedImageData.format = requestedFormat;
	compressedImageData.size = mipmappedImageData.size;
	
	glm::uvec2 size = mipmappedImageData.size;
	for (const ImageLevel& level : mipmappedImageData.levels)
	{
		if (size.x % vk::blockExtent(requestedFormat)[0] != 0 ||
			size.y % vk::blockExtent(requestedFormat)[1] != 0)
		{
			break;
		}
		
		glm::uvec2 blockCount(
			size.x / vk::blockExtent(requestedFormat)[0],
			size.y / vk::blockExtent(requestedFormat)[1]
		);
		
		ImageLevel& compressedLevel = compressedImageData.levels.emplace_back();
		compressedLevel.data.resize(blockCount.x * blockCount.y * vk::blockSize(requestedFormat));
		
		switch (requestedFormat)
		{
			case vk::Format::eBc7SrgbBlock:
				compressLevelBC7(level, compressedLevel, size, blockCount);
				break;
			case vk::Format::eBc5UnormBlock:
				compressLevelBC5(level, compressedLevel, size, blockCount);
				break;
			case vk::Format::eBc4UnormBlock:
				compressLevelBC4(level, compressedLevel, size, blockCount);
				break;
			default:
				throw;
		}
		
		size /= 2;
	}
	
	return compressedImageData;
}

static ImageData processImage(const std::filesystem::path& input, const std::filesystem::path& output, ImageType type)
{
	StbImage::Channels requiredChannels;
	StbImage::BitDepth maxBitDepth;
	switch (type)
	{
		case ImageType::ColorSrgb:
			requiredChannels = StbImage::Channels::eRedGreenBlueAlpha;
			maxBitDepth = StbImage::BitDepth::e8;
			break;
		case ImageType::NormalMap:
			requiredChannels = StbImage::Channels::eRedGreenBlue;
			maxBitDepth = StbImage::BitDepth::e8;
			break;
		case ImageType::Grayscale:
			requiredChannels = StbImage::Channels::eGrey;
			maxBitDepth = StbImage::BitDepth::e8;
			break;
		default:
			throw;
	}
	
	StbImage image(input, requiredChannels, maxBitDepth);

	if (!image.isValid())
	{
		throw std::runtime_error(std::format("Unable to load image {} from disk", input.generic_string()));
	}
	
	vk::Format mipmapGenFormat;
	vk::Format compressionFormat;
	switch (type)
	{
		case ImageType::ColorSrgb:
			switch (image.getBitsPerChannel())
			{
				case 8:
					mipmapGenFormat = vk::Format::eR8G8B8A8Srgb;
					compressionFormat = vk::Format::eBc7SrgbBlock;
					break;
				default:
					throw;
			}
			break;
		case ImageType::NormalMap:
			switch (image.getBitsPerChannel())
			{
				case 8:
					mipmapGenFormat = vk::Format::eR8G8Unorm;
					compressionFormat = vk::Format::eBc5UnormBlock;
					break;
				default:
					throw;
			}
			break;
		case ImageType::Grayscale:
			switch (image.getBitsPerChannel())
			{
				case 8:
					mipmapGenFormat = vk::Format::eR8Unorm;
					compressionFormat = vk::Format::eBc4UnormBlock;
					break;
				default:
					throw;
			}
			break;
		default:
			throw;
	}
	
	std::vector<std::byte> convertedData;
	std::span<const std::byte> data;
	switch (type)
	{
		case ImageType::ColorSrgb:
			data = {image.getPtr(), image.getByteSize()};
			break;
		case ImageType::NormalMap:
			convertedData = convertRgbToRg({image.getPtr(), image.getByteSize()}, 1);
			data = convertedData;
			break;
		case ImageType::Grayscale:
			data = {image.getPtr(), image.getByteSize()};
			break;
	}
	
	ImageData mipmappedImageData = genMipmaps(mipmapGenFormat, image.getSize(), data);
	
	ImageData compressedImageData = compressTexture(mipmappedImageData,	compressionFormat);
	
	writeProcessedImage(output, compressedImageData);
	
	return compressedImageData;
}

ImageData ImageProcessor::readImageData(std::string_view path, ImageType type, std::string_view cachePath)
{
	std::filesystem::path absolutePath = FileHelper::getAssetDirectoryPath() / path;
	std::filesystem::path cacheAbsolutePath = FileHelper::getCacheAssetDirectoryPath() / cachePath;

	ImageData imageData;
	
	if (std::filesystem::exists(cacheAbsolutePath))
	{
		Logger::info(std::format("Loading image {} with type {} from cache", path, magic_enum::enum_name(type)));
		if (!readProcessedImage(cacheAbsolutePath, imageData))
		{
			Logger::warning(std::format("Cannot parse cached image {} with type {}. Reprocessing...", path, magic_enum::enum_name(type)));
			std::filesystem::remove(cacheAbsolutePath);
			imageData = processImage(absolutePath, cacheAbsolutePath, type);
			Logger::info(std::format("Image {} with type {} reprocessed succesfully", path, magic_enum::enum_name(type)));
		}
	}
	else
	{
		Logger::info(std::format("Processing image {} with type {}", path, magic_enum::enum_name(type)));
		imageData = processImage(absolutePath, cacheAbsolutePath, type);
		Logger::info(std::format("Image {} with type {} processed succesfully", path, magic_enum::enum_name(type)));
	}
	
	return imageData;
}