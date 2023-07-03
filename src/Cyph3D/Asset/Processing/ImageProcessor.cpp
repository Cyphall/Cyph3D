#include "ImageProcessor.h"

#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/StbImage.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKComputePipeline.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/GLSL_types.h"

#include <filesystem>
#include <array>
#include <magic_enum.hpp>
#include <vulkan/vulkan_format_traits.hpp>
#include <bc7enc.h>
#include <rgbcx.h>

struct PushConstantData
{
	GLSL_bool srgb;
	GLSL_uint reduceMode;
};

static void writeProcessedImage(const std::filesystem::path& path, const ImageData& imageData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file = FileHelper::openFileForWriting(path);

	uint8_t version = 4;
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
	std::ifstream file = FileHelper::openFileForReading(path);

	uint8_t version;
	FileHelper::read(file, &version);

	if (version != 4)
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

static void compressLevelBC4(const ImageLevel& uncompressedLevel, ImageLevel& compressedLevel, glm::uvec2 inputSize, glm::uvec2 outputSize)
{
	glm::uvec2 inputPixelOffset{
		1,
		inputSize.x * 1
	};
	
	glm::uvec2 inputBlockOffset = inputPixelOffset * glm::uvec2(4, 4);
	
	std::array<uint8_t, 16> blockInput{};
	std::array<uint8_t, 8> blockOutput{};
	
	std::byte* outputPtr = compressedLevel.data.data();
	for (int blockY = 0; blockY < outputSize.y; blockY++)
	{
		for (int blockX = 0; blockX < outputSize.x; blockX++)
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
	
	std::array<uint8_t, 32> blockInput{};
	std::array<uint8_t, 16> blockOutput{};
	
	std::byte* outputPtr = compressedLevel.data.data();
	for (int blockY = 0; blockY < outputSize.y; blockY++)
	{
		for (int blockX = 0; blockX < outputSize.x; blockX++)
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
		for (int blockX = 0; blockX < outputSize.x; blockX++)
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

ImageProcessor::ImageProcessor()
{
	VKDescriptorSetLayoutInfo descriptorSetLayoutInfo(true);
	descriptorSetLayoutInfo.addBinding(vk::DescriptorType::eStorageImage, 1);
	descriptorSetLayoutInfo.addBinding(vk::DescriptorType::eStorageImage, 1);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), descriptorSetLayoutInfo);
	
	VKPipelineLayoutInfo pipelineLayoutInfo;
	pipelineLayoutInfo.addDescriptorSetLayout(_descriptorSetLayout);
	pipelineLayoutInfo.setPushConstantLayout<PushConstantData>();
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), pipelineLayoutInfo);
	
	VKComputePipelineInfo computePipelineInfo(
		_pipelineLayout,
		"resources/shaders/internal/asset processing/gen mipmap.comp");
	
	_pipeline = VKComputePipeline::create(Engine::getVKContext(), computePipelineInfo);
	
	rgbcx::init();
}

ImageData ImageProcessor::readImageData(AssetManagerWorkerData& workerData, std::string_view path, ImageType type, std::string_view cachePath)
{
	std::filesystem::path absolutePath = FileHelper::getAssetDirectoryPath() / path;
	std::filesystem::path cacheAbsolutePath = FileHelper::getCacheAssetDirectoryPath() / cachePath;

	ImageData imageData;
	
	if (std::filesystem::exists(cacheAbsolutePath))
	{
		Logger::info(std::format("Loading image [{} ({})] from cache...", path, magic_enum::enum_name(type)));
		if (readProcessedImage(cacheAbsolutePath, imageData))
		{
			Logger::info(std::format("Image [{} ({})] loaded from cache succesfully", path, magic_enum::enum_name(type)));
		}
		else
		{
			Logger::warning(std::format("Could not load image [{} ({})] from cache. Reprocessing...", path, magic_enum::enum_name(type)));
			std::filesystem::remove(cacheAbsolutePath);
			imageData = processImage(workerData, absolutePath, cacheAbsolutePath, type);
			Logger::info(std::format("Image [{} ({})] reprocessed succesfully", path, magic_enum::enum_name(type)));
		}
	}
	else
	{
		Logger::info(std::format("Processing image [{} ({})]", path, magic_enum::enum_name(type)));
		imageData = processImage(workerData, absolutePath, cacheAbsolutePath, type);
		Logger::info(std::format("Image [{} ({})] processed succesfully", path, magic_enum::enum_name(type)));
	}
	
	return imageData;
}

ImageData ImageProcessor::processImage(AssetManagerWorkerData& workerData, const std::filesystem::path& input, const std::filesystem::path& output, ImageType type)
{
	StbImage::Channels requiredChannels;
	StbImage::BitDepthFlags supportedBitDepth;
	switch (type)
	{
		case ImageType::ColorSrgb:
			requiredChannels = StbImage::Channels::eRedGreenBlueAlpha;
			supportedBitDepth = StbImage::BitDepthFlags::e8;
			break;
		case ImageType::NormalMap:
			requiredChannels = StbImage::Channels::eRedGreenBlue;
			supportedBitDepth = StbImage::BitDepthFlags::e8;
			break;
		case ImageType::Grayscale:
			requiredChannels = StbImage::Channels::eGrey;
			supportedBitDepth = StbImage::BitDepthFlags::e8;
			break;
		case ImageType::Skybox:
			requiredChannels = StbImage::Channels::eRedGreenBlueAlpha;
			supportedBitDepth = StbImage::BitDepthFlags::e8 | StbImage::BitDepthFlags::e32;
			break;
		default:
			throw;
	}
	
	StbImage image(input, requiredChannels, supportedBitDepth);
	
	if (!image.isValid())
	{
		throw std::runtime_error(std::format("Unable to load image {} from disk", input.generic_string()));
	}
	
	vk::Format mipmapGenFormat;
	bool isMipmapGenFormatSrgb;
	vk::Format compressionFormat;
	switch (type)
	{
		case ImageType::ColorSrgb:
			switch (image.getBitsPerChannel())
			{
				case 8:
					mipmapGenFormat = vk::Format::eR8G8B8A8Unorm;
					isMipmapGenFormatSrgb = true;
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
					isMipmapGenFormatSrgb = false;
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
					isMipmapGenFormatSrgb = false;
					compressionFormat = vk::Format::eBc4UnormBlock;
					break;
				default:
					throw;
			}
			break;
		case ImageType::Skybox:
			switch (image.getBitsPerChannel())
			{
				case 8:
					mipmapGenFormat = vk::Format::eR8G8B8A8Unorm;
					isMipmapGenFormatSrgb = true;
					compressionFormat = vk::Format::eBc7SrgbBlock;
					break;
				case 32:
					mipmapGenFormat = vk::Format::eR32G32B32A32Sfloat;
					isMipmapGenFormatSrgb = false;
					compressionFormat = vk::Format::eUndefined;
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
		case ImageType::Grayscale:
		case ImageType::Skybox:
			data = {image.getPtr(), image.getByteSize()};
			break;
		case ImageType::NormalMap:
			convertedData = convertRgbToRg({image.getPtr(), image.getByteSize()}, 1);
			data = convertedData;
			break;
		default:
			throw;
	}
	
	ImageData imageData = genMipmaps(workerData, mipmapGenFormat, image.getSize(), data, isMipmapGenFormatSrgb);
	
	if (compressionFormat != vk::Format::eUndefined)
	{
		imageData = compressTexture(imageData,	compressionFormat);
	}
	
	writeProcessedImage(output, imageData);
	
	return imageData;
}

ImageData ImageProcessor::genMipmaps(AssetManagerWorkerData& workerData, vk::Format format, glm::uvec2 size, std::span<const std::byte> data, bool isSrgb)
{
	// create texture
	VKImageInfo imageInfo(
		format,
		size,
		1,
		VKImage::calcMaxMipLevels(size),
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	VKPtr<VKImage> texture = VKImage::create(Engine::getVKContext(), imageInfo);
	
	// create texture views
	std::vector<VKPtr<VKImageView>> textureViews;
	for (int i = 0; i < texture->getInfo().getLevels(); i++)
	{
		VKImageViewInfo imageViewInfo(texture, vk::ImageViewType::e2D);
		imageViewInfo.setCustomLevelRange({i, i});
		
		textureViews.push_back(VKImageView::create(Engine::getVKContext(), imageViewInfo));
	}
	
	// create staging buffer
	VKBufferInfo bufferInfo(texture->getLayerByteSize(), vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCached);
	
	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), bufferInfo);
	
	// copy texture data to staging buffer
	std::copy(data.data(), data.data() + texture->getLevelByteSize(0), stagingBuffer->getHostPointer());
	
	// upload staging buffer to texture
	workerData.transferCommandBuffer->begin();
	
	workerData.transferCommandBuffer->imageMemoryBarrier(
		texture, 0, 0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferWrite,
		vk::ImageLayout::eTransferDstOptimal);
	
	workerData.transferCommandBuffer->copyBufferToImage(stagingBuffer, 0, texture, 0, 0);
	
	workerData.transferCommandBuffer->end();
	
	Engine::getVKContext().getTransferQueue().submit(workerData.transferCommandBuffer, nullptr, nullptr);
	
	workerData.transferCommandBuffer->waitExecution();
	workerData.transferCommandBuffer->reset();
	
	// generate mipmaps
	workerData.computeCommandBuffer->begin();
	
	workerData.computeCommandBuffer->bindPipeline(_pipeline);
	
	PushConstantData pushConstantData{
		.srgb = isSrgb,
		.reduceMode = 0
	};
	workerData.computeCommandBuffer->pushConstants(pushConstantData);
	
	workerData.computeCommandBuffer->imageMemoryBarrier(
		texture, 0, 0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral);
	
	for (int i = 1; i < texture->getInfo().getLevels(); i++)
	{
		workerData.computeCommandBuffer->imageMemoryBarrier(
			texture, 0, i-1,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageWrite,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageRead);
		
		workerData.computeCommandBuffer->imageMemoryBarrier(
			texture, 0, i,
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageWrite,
			vk::ImageLayout::eGeneral);
		
		workerData.computeCommandBuffer->pushDescriptor(0, 0, textureViews[i-1]);
		workerData.computeCommandBuffer->pushDescriptor(0, 1, textureViews[i]);
		
		glm::uvec2 dstSize = texture->getSize(i);
		workerData.computeCommandBuffer->dispatch({(dstSize.x + 7) / 8, (dstSize.y + 7) / 8, 1});
	}
	workerData.computeCommandBuffer->end();
	
	Engine::getVKContext().getComputeQueue().submit(workerData.computeCommandBuffer, nullptr, nullptr);
	
	workerData.computeCommandBuffer->waitExecution();
	workerData.computeCommandBuffer->reset();
	
	// download texture to staging buffer
	workerData.transferCommandBuffer->begin();
	
	vk::DeviceSize bufferOffset = texture->getLevelByteSize(0);
	for (uint32_t i = 1; i < texture->getInfo().getLevels(); i++)
	{
		workerData.transferCommandBuffer->imageMemoryBarrier(
			texture, 0, i,
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferRead,
			vk::ImageLayout::eTransferSrcOptimal);
		
		workerData.transferCommandBuffer->copyImageToBuffer(texture, 0, i, stagingBuffer, bufferOffset);
		bufferOffset += texture->getLevelByteSize(i);
	}
	
	workerData.transferCommandBuffer->end();
	
	Engine::getVKContext().getTransferQueue().submit(workerData.transferCommandBuffer, nullptr, nullptr);
	
	workerData.transferCommandBuffer->waitExecution();
	workerData.transferCommandBuffer->reset();
	
	ImageData imageData;
	imageData.format = format;
	imageData.size = size;
	imageData.levels.resize(texture->getInfo().getLevels());
	
	std::byte* ptr = stagingBuffer->getHostPointer();
	for (uint32_t i = 0; i < imageData.levels.size(); i++)
	{
		imageData.levels[i].data.resize(texture->getLevelByteSize(i));
		
		std::memcpy(imageData.levels[i].data.data(), ptr, imageData.levels[i].data.size());
		
		ptr += imageData.levels[i].data.size();
	}
	
	return imageData;
}