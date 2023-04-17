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

static void writeProcessedImage(const std::filesystem::path& path, const ImageData& imageData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file(path, std::ios::out | std::ios::binary);

	uint8_t version = 2;
	FileHelper::write(file, &version);
	
	FileHelper::write(file, &imageData.format);

	FileHelper::write(file, &imageData.size);
	
	FileHelper::write(file, imageData.data);
}

static bool readProcessedImage(const std::filesystem::path& path, ImageData& imageData)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);

	uint8_t version;
	FileHelper::read(file, &version);

	if (version != 2)
	{
		return false;
	}
	
	FileHelper::read(file, &imageData.format);
	
	FileHelper::read(file, &imageData.size);
	
	FileHelper::read(file, imageData.data);

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

static ImageData processImage(const std::filesystem::path& input, const std::filesystem::path& output, ImageType type)
{
	ImageData imageData;
	
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
			maxBitDepth = StbImage::BitDepth::e16;
			break;
		case ImageType::Grayscale:
			requiredChannels = StbImage::Channels::eGrey;
			maxBitDepth = StbImage::BitDepth::e16;
			break;
		default:
			throw;
	}
	
	StbImage image(input, requiredChannels, maxBitDepth);

	if (!image.isValid())
	{
		throw std::runtime_error(std::format("Unable to load image {} from disk", input.generic_string()));
	}
	
	imageData.size = image.getSize();
	//TODO: figure out texture compression in Vulkan
	vk::Format format;
	switch (type)
	{
		case ImageType::ColorSrgb:
			switch (image.getBitsPerChannel())
			{
				case 8:
					format = vk::Format::eR8G8B8A8Srgb;
					break;
				default:
					throw;
			}
			break;
		case ImageType::NormalMap:
			switch (image.getBitsPerChannel())
			{
				case 8:
					format = vk::Format::eR8G8Unorm;
					break;
				case 16:
					format = vk::Format::eR16G16Unorm;
					break;
				default:
					throw;
			}
			break;
		case ImageType::Grayscale:
			switch (image.getBitsPerChannel())
			{
				case 8:
					format = vk::Format::eR8Unorm;
					break;
				case 16:
					format = vk::Format::eR16Unorm;
					break;
				default:
					throw;
			}
			break;
		default:
			throw;
	}
	
	imageData.format = format;
	
	VKPtr<VKImage> texture = VKImage::create(
		Engine::getVKContext(),
		format,
		imageData.size,
		1,
		VKImage::calcMaxMipLevels(imageData.size),
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc,
		vk::ImageAspectFlagBits::eColor,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(
		Engine::getVKContext(),
		texture->getLayerByteSize(),
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostCached);
	
	std::vector<std::byte> convertedData;
	const std::byte* data;
	switch (type)
	{
		case ImageType::ColorSrgb:
			data = image.getPtr();
			break;
		case ImageType::NormalMap:
			if (image.getBitsPerChannel() == 8)
			{
				convertedData = convertRgbToRg({image.getPtr(), image.getByteSize()}, 1);
			}
			else if (image.getBitsPerChannel() == 16)
			{
				convertedData = convertRgbToRg({image.getPtr(), image.getByteSize()}, 2);
			}
			else
			{
				throw;
			}
			data = convertedData.data();
			break;
		case ImageType::Grayscale:
			data = image.getPtr();
			break;
	}
	
	std::byte* ptr = stagingBuffer->map();
	std::copy(data, data + texture->getLevelByteSize(0), ptr);
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
	
	ptr = stagingBuffer->map();
	vk::DeviceSize byteSize = texture->getLayerByteSize();
	imageData.data.resize(byteSize);
	std::copy(ptr, ptr + byteSize, imageData.data.begin());
	stagingBuffer->unmap();
	
	writeProcessedImage(output, imageData);
	
	return imageData;
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