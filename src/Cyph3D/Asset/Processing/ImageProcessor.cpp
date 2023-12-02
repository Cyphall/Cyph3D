#include "ImageProcessor.h"

#include "Cyph3D/Asset/Processing/ImageCompressor.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/StbImage.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKComputePipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"

#include <filesystem>
#include <half.hpp>
#include <magic_enum.hpp>

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
		FileHelper::write(file, imageData.levels[i]);
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

	uint32_t levels;
	FileHelper::read(file, &levels);

	imageData.levels.resize(levels);
	for (uint32_t i = 0; i < levels; i++)
	{
		FileHelper::read(file, imageData.levels[i]);
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

static std::vector<std::byte> convertFloatToHalf(std::span<const std::byte> input)
{
	std::vector<std::byte> output(input.size() / 2);

	for (size_t i = 0; i < input.size() / sizeof(float); i++)
	{
		float dataF;
		std::memcpy(&dataF, input.data() + i * sizeof(float), sizeof(float));

		half_float::half dataH(std::clamp(dataF, -65000.0f, 65000.0f));
		std::memcpy(output.data() + i * sizeof(half_float::half), &dataH, sizeof(half_float::half));
	}

	return output;
}

ImageData compressTexture(const ImageData& mipmappedImageData, vk::Format requestedFormat)
{
	ImageData compressedImageData;
	compressedImageData.format = requestedFormat;
	compressedImageData.size = mipmappedImageData.size;

	glm::uvec2 size = mipmappedImageData.size;
	for (const std::vector<std::byte>& level : mipmappedImageData.levels)
	{
		std::vector<std::byte> compressedData;
		if (!ImageCompressor::tryCompressImage(level, size, requestedFormat, compressedData))
			break;

		compressedImageData.levels.emplace_back(std::move(compressedData));

		size = glm::max(size / 2u, glm::uvec2(1, 1));
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
		"resources/shaders/internal/asset processing/gen mipmap.comp"
	);

	_pipeline = VKComputePipeline::create(Engine::getVKContext(), computePipelineInfo);
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
			mipmapGenFormat = vk::Format::eR16G16B16A16Sfloat;
			isMipmapGenFormatSrgb = false;
			compressionFormat = vk::Format::eBc6HUfloatBlock;
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
	if (type == ImageType::NormalMap)
	{
		convertedData = convertRgbToRg({image.getPtr(), image.getByteSize()}, 1);
		data = convertedData;
	}
	else if (type == ImageType::Skybox && image.getBitsPerChannel() == 32)
	{
		convertedData = convertFloatToHalf({image.getPtr(), image.getByteSize()});
		data = convertedData;
	}
	else
	{
		data = {image.getPtr(), image.getByteSize()};
	}

	ImageData imageData = genMipmaps(workerData, mipmapGenFormat, image.getSize(), data, isMipmapGenFormatSrgb);

	if (compressionFormat != vk::Format::eUndefined)
	{
		imageData = compressTexture(imageData, compressionFormat);
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
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);

	VKPtr<VKImage> texture = VKImage::create(Engine::getVKContext(), imageInfo);

	// create staging buffer
	VKBufferInfo bufferInfo(texture->getLayerByteSize(), vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);

	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), bufferInfo);

	// copy texture data to staging buffer
	std::copy(data.data(), data.data() + texture->getLevelByteSize(0), stagingBuffer->getHostPointer());

	// upload staging buffer to texture
	workerData.transferCommandBuffer->begin();

	workerData.transferCommandBuffer->imageMemoryBarrier(
		texture,
		{0, 0},
		{0, 0},
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferWrite,
		vk::ImageLayout::eTransferDstOptimal
	);

	workerData.transferCommandBuffer->copyBufferToImage(stagingBuffer, 0, texture, 0, 0);

	workerData.transferCommandBuffer->end();

	Engine::getVKContext().getTransferQueue().submit(workerData.transferCommandBuffer, {}, {});

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
		texture,
		{0, 0},
		{0, 0},
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageRead,
		vk::ImageLayout::eGeneral
	);

	for (int i = 1; i < texture->getInfo().getLevels(); i++)
	{
		workerData.computeCommandBuffer->imageMemoryBarrier(
			texture,
			{0, 0},
			{i, i},
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageWrite,
			vk::ImageLayout::eGeneral
		);

		workerData.computeCommandBuffer->pushDescriptor(
			0,
			0,
			texture,
			vk::ImageViewType::e2D,
			{0, 0},
			{i - 1, i - 1},
			texture->getInfo().getFormat()
		);

		workerData.computeCommandBuffer->pushDescriptor(
			0,
			1,
			texture,
			vk::ImageViewType::e2D,
			{0, 0},
			{i, i},
			texture->getInfo().getFormat()
		);

		glm::uvec2 dstSize = texture->getSize(i);
		workerData.computeCommandBuffer->dispatch({(dstSize.x + 7) / 8, (dstSize.y + 7) / 8, 1});

		workerData.computeCommandBuffer->imageMemoryBarrier(
			texture,
			{0, 0},
			{i, i},
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageWrite,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageRead,
			vk::ImageLayout::eGeneral
		);
	}
	workerData.computeCommandBuffer->end();

	Engine::getVKContext().getComputeQueue().submit(workerData.computeCommandBuffer, {}, {});

	workerData.computeCommandBuffer->waitExecution();
	workerData.computeCommandBuffer->reset();

	// download texture to staging buffer
	workerData.transferCommandBuffer->begin();

	vk::DeviceSize bufferOffset = texture->getLevelByteSize(0);
	for (uint32_t i = 1; i < texture->getInfo().getLevels(); i++)
	{
		workerData.transferCommandBuffer->imageMemoryBarrier(
			texture,
			{0, 0},
			{i, i},
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eCopy,
			vk::AccessFlagBits2::eTransferRead,
			vk::ImageLayout::eTransferSrcOptimal
		);

		workerData.transferCommandBuffer->copyImageToBuffer(texture, 0, i, stagingBuffer, bufferOffset);
		bufferOffset += texture->getLevelByteSize(i);
	}

	workerData.transferCommandBuffer->end();

	Engine::getVKContext().getTransferQueue().submit(workerData.transferCommandBuffer, {}, {});

	workerData.transferCommandBuffer->waitExecution();
	workerData.transferCommandBuffer->reset();

	ImageData imageData;
	imageData.format = format;
	imageData.size = size;
	imageData.levels.resize(texture->getInfo().getLevels());

	std::byte* ptr = stagingBuffer->getHostPointer();
	for (uint32_t i = 0; i < imageData.levels.size(); i++)
	{
		imageData.levels[i].resize(texture->getLevelByteSize(i));

		std::memcpy(imageData.levels[i].data(), ptr, imageData.levels[i].size());

		ptr += imageData.levels[i].size();
	}

	return imageData;
}