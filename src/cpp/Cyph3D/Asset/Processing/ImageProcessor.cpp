#include "ImageProcessor.h"

#include <Cyph3D/Asset/AssetManagerWorkerData.h>
#include <Cyph3D/Asset/Processing/ImageCompressor.h>
#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/StbImage.h>

#include <CyphGPU/Buffer.hpp>
#include <CyphGPU/CommandContext.hpp>
#include <CyphGPU/CommandRecorder.hpp>
#include <CyphGPU/ComputeShaderState.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/Image.hpp>
#include <filesystem>
#include <half.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

namespace
{
void writeProcessedImage(const std::filesystem::path& path, const c3d::ImageData& imageData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file = c3d::FileHelper::openFileForWriting(path);

	uint8_t version = 5;
	c3d::FileHelper::write(file, &version);

	c3d::FileHelper::write(file, &imageData.format);

	c3d::FileHelper::write(file, &imageData.extent);

	uint32_t levels = imageData.levels.size();
	c3d::FileHelper::write(file, &levels);

	for (uint32_t i = 0; i < levels; i++)
	{
		c3d::FileHelper::write(file, imageData.levels[i]);
	}
}

bool readProcessedImage(const std::filesystem::path& path, c3d::ImageData& imageData)
{
	std::ifstream file = c3d::FileHelper::openFileForReading(path);

	uint8_t version;
	c3d::FileHelper::read(file, &version);

	if (version != 5)
	{
		return false;
	}

	c3d::FileHelper::read(file, &imageData.format);

	c3d::FileHelper::read(file, &imageData.extent);

	uint32_t levels;
	c3d::FileHelper::read(file, &levels);

	imageData.levels.resize(levels);
	for (uint32_t i = 0; i < levels; i++)
	{
		c3d::FileHelper::read(file, imageData.levels[i]);
	}

	return true;
}

std::vector<std::byte> convertRgbToRg(std::span<const std::byte> input, int bytesPerChannel)
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

std::vector<std::byte> convertFloatToHalf(std::span<const std::byte> input)
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

c3d::ImageData compressImage(const c3d::ImageData& mipmappedImageData, vk::Format compressionFormat)
{
	c3d::ImageData compressedImageData;
	compressedImageData.format = compressionFormat;
	compressedImageData.extent = mipmappedImageData.extent;

	glm::uvec2 extent = mipmappedImageData.extent;
	for (const std::vector<std::byte>& srcStorage : mipmappedImageData.levels)
	{
		auto& dstStorage = compressedImageData.levels.emplace_back();
		dstStorage.resize(cgpu::calcImageByteSize(compressionFormat, glm::uvec3{extent, 1}, 1));

		c3d::ImageCompressor::compressImage(
			srcStorage,
			extent,
			compressionFormat,
			dstStorage
		);

		extent = glm::max(extent >> 1u, glm::uvec2{1, 1});
	}

	return compressedImageData;
}
}

c3d::ImageData c3d::ImageProcessor::readImageData(std::string_view path, ImageType type, std::string_view cachePath)
{
	std::filesystem::path absolutePath = FileHelper::getAssetDirectoryPath() / path;
	std::filesystem::path cacheAbsolutePath = FileHelper::getCacheAssetDirectoryPath() / cachePath;

	ImageData imageData;

	if (std::filesystem::exists(cacheAbsolutePath))
	{
		spdlog::info("Loading image [{} ({})] from cache...", path, magic_enum::enum_name(type));
		if (readProcessedImage(cacheAbsolutePath, imageData))
		{
			spdlog::info("Image [{} ({})] loaded from cache succesfully", path, magic_enum::enum_name(type));
		}
		else
		{
			spdlog::warn("Could not load image [{} ({})] from cache. Reprocessing...", path, magic_enum::enum_name(type));
			std::filesystem::remove(cacheAbsolutePath);
			imageData = processImage(absolutePath, cacheAbsolutePath, type);
			spdlog::info("Image [{} ({})] reprocessed succesfully", path, magic_enum::enum_name(type));
		}
	}
	else
	{
		spdlog::info("Processing image [{} ({})]", path, magic_enum::enum_name(type));
		imageData = processImage(absolutePath, cacheAbsolutePath, type);
		spdlog::info("Image [{} ({})] processed succesfully", path, magic_enum::enum_name(type));
	}

	return imageData;
}

c3d::ImageData c3d::ImageProcessor::processImage(const std::filesystem::path& input, const std::filesystem::path& output, ImageType type)
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
	case ImageType::Skybox:
		switch (image.getBitsPerChannel())
		{
		case 8:
			mipmapGenFormat = vk::Format::eR8G8B8A8Srgb;
			compressionFormat = vk::Format::eBc7SrgbBlock;
			break;
		case 32:
			mipmapGenFormat = vk::Format::eR16G16B16A16Sfloat;
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

	ImageData imageData = genMipmaps(mipmapGenFormat, image.getSize(), data);

	if (compressionFormat != vk::Format::eUndefined)
	{
		imageData = compressImage(imageData, compressionFormat);
	}

	writeProcessedImage(output, imageData);

	return imageData;
}

c3d::ImageData c3d::ImageProcessor::genMipmaps(vk::Format format, glm::uvec2 extent, std::span<const std::byte> data)
{
	// create image
	auto image = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = "Image processing image",
			.format = format,
			.extent = {extent, 1},
			.usages =
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eTransferSrc,
			.levels = cgpu::calcImageMaxLevelCount({extent, 1}),
		}
	);

	// create staging buffer
	cgpu::BufferPtr stagingBuffer = cgpu::Buffer::create(
		Engine::getDeviceSession(),
		{
			.name = "Image processing staging buffer",
			.size = image->calcByteSize({0, image->getDesc().levels}, 1),
			.usages =
				vk::BufferUsageFlagBits2::eTransferSrc |
				vk::BufferUsageFlagBits2::eTransferDst,
			.memory_type = cgpu::MemoryType::eCPUCached,
		}
	);

	// copy image data to staging buffer
	std::copy_n(data.data(), data.size_bytes(), stagingBuffer->getHostPtr());

	// upload staging buffer to image
	{
		auto commandRecorder = assetCommandContext->createRecorder(Engine::getDeviceSession()->getAsyncTransferQueue());

		commandRecorder.copyBufferToImage({
			.src_buffer = stagingBuffer,
			.dst_image = image,
			.ranges = {{
				{
					.src = {{.byte_range = {{0, image->calcByteSize({0, 1}, 1)}}}},
				},
			}},
		});

		commandRecorder.submit();
	}

	// generate mipmaps
	{
		auto commandRecorder = assetCommandContext->createRecorder(Engine::getDeviceSession()->getAsyncGraphicsQueue());

		for (uint32_t i = 1; i < image->getDesc().levels; i++)
		{
			commandRecorder.blit({
				.src_image = image,
				.dst_image = image,
				.filter = vk::Filter::eLinear,
				.ranges = {{
					{
						.src = {{.level = i - 1}},
						.dst = {{.level = i - 0}},
					},
				}},
			});
		}

		commandRecorder.submit();
	}

	// download generated image levels to staging buffer
	{
		auto commandRecorder = assetCommandContext->createRecorder(Engine::getDeviceSession()->getAsyncTransferQueue());

		std::vector<cgpu::CommandRecorder::CopyImageToBufferParams::Range> ranges;
		vk::DeviceSize bufferOffset = image->calcByteSize({0, 1}, 1);
		for (uint32_t level = 1; level < image->getDesc().levels; level++)
		{
			size_t size = image->calcByteSize({level, 1}, 1);

			ranges.push_back({
				.src = {{
					.level = level,
				}},
				.dst = {{
					.byte_range = {{bufferOffset, size}},
				}},
			});

			bufferOffset += size;
		}

		commandRecorder.copyImageToBuffer({
			.src_image = image,
			.dst_buffer = stagingBuffer,
			.ranges = ranges,
		});

		commandRecorder.submit().waitFinished();
	}

	assetCommandContext->finish();

	ImageData imageData;
	imageData.format = format;
	imageData.extent = extent;
	imageData.levels.resize(image->getDesc().levels);

	std::byte* ptr = stagingBuffer->getHostPtr();
	for (uint32_t i = 0; i < imageData.levels.size(); i++)
	{
		imageData.levels[i].resize(image->calcByteSize({i, 1}, 1));

		std::copy_n(ptr, imageData.levels[i].size(), imageData.levels[i].data());
		ptr += imageData.levels[i].size();
	}

	return imageData;
}
