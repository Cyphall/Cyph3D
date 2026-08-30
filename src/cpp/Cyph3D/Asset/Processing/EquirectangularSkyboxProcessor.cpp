#include "EquirectangularSkyboxProcessor.h"

#include <Cyph3D/Asset/AssetManagerWorkerData.h>
#include <Cyph3D/Asset/Processing/ImageCompressor.h>
#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/StbImage.h>

#include <array>
#include <CyphGPU/Buffer.hpp>
#include <CyphGPU/CommandContext.hpp>
#include <CyphGPU/CommandRecorder.hpp>
#include <CyphGPU/ComputePassContext.hpp>
#include <CyphGPU/ComputeShaderState.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/Image.hpp>
#include <CyphGPU/Sampler.hpp>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <half.hpp>
#include <spdlog/spdlog.h>

namespace
{
void writeProcessedEquirectangularSkybox(const std::filesystem::path& path, const c3d::EquirectangularSkyboxData& equirectangularSkyboxData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file = c3d::FileHelper::openFileForWriting(path);

	uint8_t version = 2;
	c3d::FileHelper::write(file, &version);

	c3d::FileHelper::write(file, &equirectangularSkyboxData.format);

	c3d::FileHelper::write(file, &equirectangularSkyboxData.extent);

	uint32_t levels = equirectangularSkyboxData.levels.size();
	c3d::FileHelper::write(file, &levels);

	for (uint32_t i = 0; i < levels; i++)
	{
		c3d::FileHelper::write(file, equirectangularSkyboxData.levels[i]);
	}
}

bool readProcessedEquirectangularSkybox(const std::filesystem::path& path, c3d::EquirectangularSkyboxData& equirectangularSkyboxData)
{
	std::ifstream file = c3d::FileHelper::openFileForReading(path);

	uint8_t version;
	c3d::FileHelper::read(file, &version);

	if (version != 2)
	{
		return false;
	}

	c3d::FileHelper::read(file, &equirectangularSkyboxData.format);

	c3d::FileHelper::read(file, &equirectangularSkyboxData.extent);

	uint32_t levels;
	c3d::FileHelper::read(file, &levels);

	equirectangularSkyboxData.levels.resize(levels);
	for (uint32_t i = 0; i < levels; i++)
	{
		c3d::FileHelper::read(file, equirectangularSkyboxData.levels[i]);
	}

	return true;
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

c3d::EquirectangularSkyboxData compressImage(const c3d::EquirectangularSkyboxData& mipmappedEquirectangularSkyboxData, vk::Format compressionFormat)
{
	c3d::EquirectangularSkyboxData compressedEquirectangularSkyboxData;
	compressedEquirectangularSkyboxData.format = compressionFormat;
	compressedEquirectangularSkyboxData.extent = mipmappedEquirectangularSkyboxData.extent;

	glm::uvec2 extent = mipmappedEquirectangularSkyboxData.extent;
	for (const std::vector<std::byte>& srcStorage : mipmappedEquirectangularSkyboxData.levels)
	{
		auto& dstStorage = compressedEquirectangularSkyboxData.levels.emplace_back();
		dstStorage.resize(cgpu::calcImageByteSize(compressionFormat, glm::uvec3{extent, 1}, 6));

		size_t srcLayerByteSize = srcStorage.size() / 6;
		size_t dstLayerByteSize = dstStorage.size() / 6;
		for (uint32_t face = 0; face < 6; face++)
		{
			c3d::ImageCompressor::compressImage(
				{srcStorage.data() + srcLayerByteSize * face, srcLayerByteSize},
				extent,
				compressionFormat,
				{dstStorage.data() + dstLayerByteSize * face, dstLayerByteSize}
			);
		}

		extent = glm::max(extent >> 1u, glm::uvec2{1, 1});
	}

	return compressedEquirectangularSkyboxData;
}

cgpu::ImagePtr uploadEquirectangularImage(vk::Format format, glm::uvec2 size, std::span<const std::byte> data)
{
	// create equirectangular image
	auto image = cgpu::Image::create(
		c3d::Engine::getDeviceSession(),
		{
			.name = "Equirectangular skybox processing equirectangular image",
			.format = format,
			.extent = {size, 1},
			.usages =
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eSampled,
		}
	);

	// create staging buffer
	cgpu::BufferPtr stagingBuffer = cgpu::Buffer::create(
		c3d::Engine::getDeviceSession(),
		{
			.name = "Equirectangular skybox processing upload staging buffer",
			.size = image->calcByteSize({0, 1}, 1),
			.usages = vk::BufferUsageFlagBits2::eTransferSrc,
			.memory_type = cgpu::MemoryType::eCPUUncached,
		}
	);

	// copy image data to staging buffer
	std::copy_n(data.data(), stagingBuffer->getDesc().size, stagingBuffer->getHostPtr());

	// upload staging buffer to image
	{
		auto commandRecorder = c3d::assetCommandContext->createRecorder(c3d::Engine::getDeviceSession()->getAsyncTransferQueue());

		commandRecorder.copyBufferToImage({
			.src_buffer = stagingBuffer,
			.dst_image = image,
		});

		commandRecorder.submit();
	}

	return image;
}

c3d::EquirectangularSkyboxData downloadCubemapImage(const cgpu::ImagePtr& cubemapImage)
{
	// create staging buffer
	cgpu::BufferPtr stagingBuffer = cgpu::Buffer::create(
		c3d::Engine::getDeviceSession(),
		{
			.name = "Equirectangular skybox processing download staging buffer",
			.size = cubemapImage->calcByteSize({0, cubemapImage->getDesc().levels}, 6),
			.usages = vk::BufferUsageFlagBits2::eTransferDst,
			.memory_type = cgpu::MemoryType::eCPUCached,
		}
	);

	// download generated image levels to staging buffer
	{
		auto commandRecorder = c3d::assetCommandContext->createRecorder(c3d::Engine::getDeviceSession()->getAsyncTransferQueue());

		std::vector<cgpu::CommandRecorder::CopyImageToBufferParams::Range> ranges;
		vk::DeviceSize bufferOffset = 0;
		for (uint32_t level = 0; level < cubemapImage->getDesc().levels; level++)
		{
			size_t size = cubemapImage->calcByteSize({level, 1}, 6);

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
			.src_image = cubemapImage,
			.dst_buffer = stagingBuffer,
			.ranges = ranges,
		});

		commandRecorder.submit().waitFinished();
	}

	c3d::assetCommandContext->finish();

	c3d::EquirectangularSkyboxData equirectangularSkyboxData;
	equirectangularSkyboxData.format = cubemapImage->getDesc().format;
	equirectangularSkyboxData.extent = cubemapImage->getDesc().extent;
	equirectangularSkyboxData.levels.resize(cubemapImage->getDesc().levels);

	std::byte* ptr = stagingBuffer->getHostPtr();
	for (uint32_t i = 0; i < equirectangularSkyboxData.levels.size(); i++)
	{
		equirectangularSkyboxData.levels[i].resize(cubemapImage->calcByteSize({i, 1}, 6));

		std::copy_n(ptr, equirectangularSkyboxData.levels[i].size(), equirectangularSkyboxData.levels[i].data());
		ptr += equirectangularSkyboxData.levels[i].size();
	}

	return equirectangularSkyboxData;
}
}

c3d::EquirectangularSkyboxProcessor::EquirectangularSkyboxProcessor()
{
	_cubemapComputeShader = cgpu::ComputeShaderState::create(
		Engine::getDeviceSession(),
		{
			.compute_shader = {.source = "Cyph3D/asset processing/gen cubemap.slang"},
		}
	);

	_cubemapSampler = cgpu::Sampler::create(
		Engine::getDeviceSession(),
		{
			.min_filter = vk::Filter::eLinear,
			.mag_filter = vk::Filter::eLinear,
			.wrapping_u = vk::SamplerAddressMode::eClampToEdge,
			.wrapping_v = vk::SamplerAddressMode::eClampToEdge,
			.wrapping_w = vk::SamplerAddressMode::eClampToEdge,
		}
	);
}

c3d::EquirectangularSkyboxData c3d::EquirectangularSkyboxProcessor::readEquirectangularSkyboxData(std::string_view path, std::string_view cachePath)
{
	std::filesystem::path absolutePath = FileHelper::getAssetDirectoryPath() / path;
	std::filesystem::path cacheAbsolutePath = FileHelper::getCacheAssetDirectoryPath() / cachePath;

	EquirectangularSkyboxData equirectangularSkyboxData;

	if (std::filesystem::exists(cacheAbsolutePath))
	{
		spdlog::info("Loading equirectangular skybox [{}] from cache...", path);
		if (readProcessedEquirectangularSkybox(cacheAbsolutePath, equirectangularSkyboxData))
		{
			spdlog::info("Equirectangular skybox [{}] loaded from cache succesfully", path);
		}
		else
		{
			spdlog::warn("Could not load equirectangular skybox [{}] from cache. Reprocessing...", path);
			std::filesystem::remove(cacheAbsolutePath);
			equirectangularSkyboxData = processEquirectangularSkybox(absolutePath, cacheAbsolutePath);
			spdlog::info("Equirectangular skybox [{}] reprocessed succesfully", path);
		}
	}
	else
	{
		spdlog::info("Processing equirectangular skybox [{}]", path);
		equirectangularSkyboxData = processEquirectangularSkybox(absolutePath, cacheAbsolutePath);
		spdlog::info("Equirectangular skybox [{}] processed succesfully", path);
	}

	return equirectangularSkyboxData;
}

c3d::EquirectangularSkyboxData c3d::EquirectangularSkyboxProcessor::processEquirectangularSkybox(const std::filesystem::path& input, const std::filesystem::path& output)
{
	StbImage::Channels requiredChannels = StbImage::Channels::eRedGreenBlueAlpha;
	StbImage::BitDepthFlags supportedBitDepth = StbImage::BitDepthFlags::e8 | StbImage::BitDepthFlags::e32;

	StbImage image(input, requiredChannels, supportedBitDepth);

	if (!image.isValid())
	{
		throw std::runtime_error(std::format("Unable to load image {} from disk", input.generic_string()));
	}

	vk::Format cubemapAndMipmapGenFormat;
	vk::Format compressionFormat;
	switch (image.getBitsPerChannel())
	{
	case 8:
		cubemapAndMipmapGenFormat = vk::Format::eR8G8B8A8Srgb;
		compressionFormat = vk::Format::eBc7SrgbBlock;
		break;
	case 32:
		cubemapAndMipmapGenFormat = vk::Format::eR16G16B16A16Sfloat;
		compressionFormat = vk::Format::eBc6HUfloatBlock;
		break;
	default:
		throw;
	}

	std::vector<std::byte> convertedData;
	std::span<const std::byte> data;
	if (image.getBitsPerChannel() == 32)
	{
		convertedData = convertFloatToHalf({image.getPtr(), image.getByteSize()});
		data = convertedData;
	}
	else
	{
		data = {image.getPtr(), image.getByteSize()};
	}

	EquirectangularSkyboxData imageData = genCubemapAndMipmaps(cubemapAndMipmapGenFormat, image.getSize(), data);

	if (compressionFormat != vk::Format::eUndefined)
	{
		imageData = compressImage(imageData, compressionFormat);
	}

	writeProcessedEquirectangularSkybox(output, imageData);

	return imageData;
}

cgpu::ImagePtr c3d::EquirectangularSkyboxProcessor::generateCubemap(vk::Format format, const cgpu::ImagePtr& equirectangularImage)
{
	// create cubemap image
	auto cubemapImage = cgpu::Image::create(
		c3d::Engine::getDeviceSession(),
		{
			.name = "Equirectangular skybox processing image",
			.format = format,
			.extent = {glm::uvec2{equirectangularImage->getDesc().extent.y / 2}, 1},
			.usages =
				vk::ImageUsageFlagBits::eStorage |
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eTransferSrc,
			.levels = cgpu::calcImageMaxLevelCount({glm::uvec2{equirectangularImage->getDesc().extent.y / 2}, 1}),
			.layers = 6,
		}
	);

	// convert equirectangular to cubemap
	{
		auto commandRecorder = assetCommandContext->createRecorder(Engine::getDeviceSession()->getAsyncComputeQueue());

		commandRecorder.computePass({
			.callback = [&](cgpu::ComputePassContext& ctx) {
				std::array views = {
					glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{+1.0f, +0.0f, +0.0f}, glm::vec3{+0.0f, +1.0f, +0.0f}),
					glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{-1.0f, +0.0f, +0.0f}, glm::vec3{+0.0f, +1.0f, +0.0f}),
					glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{+0.0f, +1.0f, +0.0f}, glm::vec3{+0.0f, +0.0f, +1.0f}),
					glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{+0.0f, -1.0f, +0.0f}, glm::vec3{+0.0f, +0.0f, -1.0f}),
					glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{+0.0f, +0.0f, -1.0f}, glm::vec3{+0.0f, +1.0f, +0.0f}),
					glm::lookAt(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{+0.0f, +0.0f, +1.0f}, glm::vec3{+0.0f, +1.0f, +0.0f}),
				};

				glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 10.0f);
				projection[1][1] *= -1;

				for (uint32_t layer = 0; layer < 6; layer++)
				{
					using namespace cgpu::shader_types;
					struct
					{
						float4x4 u_viewProjectionInv;
						Texture2D<>::Handle u_equirectangularImage;
						WTexture2D<>::Handle u_cubemapImage;
						SamplerState::Handle u_sampler;
						uint2 u_size;
						float2 u_invSizeF;
						bool u_srgb;
					} parameters{};

					parameters.u_viewProjectionInv = glm::inverse(projection * views[layer]);
					parameters.u_equirectangularImage = ctx.getSampledImageDescriptor(equirectangularImage);
					parameters.u_cubemapImage = ctx.getStorageImageDescriptor(cubemapImage, cgpu::StorageAccess::eWriteonly, {.layers = {{layer, 1}}});
					parameters.u_sampler = _cubemapSampler->getDescriptor();
					parameters.u_size = glm::uvec2{cubemapImage->getDesc().extent};
					parameters.u_invSizeF = glm::vec2{1.0f} / glm::vec2{parameters.u_size.get()};
					parameters.u_srgb = cgpu::getLinearEquivalent(format) != format;

					ctx.dispatch(_cubemapComputeShader, {parameters.u_size.get(), 1}, {8, 8, 1}, parameters);
				}
			},
		});

		commandRecorder.submit();
	}

	return cubemapImage;
}

void c3d::EquirectangularSkyboxProcessor::generateMipmaps(const cgpu::ImagePtr& cubemapImage)
{
	// generate mipmaps
	{
		auto commandRecorder = assetCommandContext->createRecorder(Engine::getDeviceSession()->getAsyncGraphicsQueue());

		for (uint32_t i = 1; i < cubemapImage->getDesc().levels; i++)
		{
			commandRecorder.blit({
				.src_image = cubemapImage,
				.dst_image = cubemapImage,
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
}

c3d::EquirectangularSkyboxData c3d::EquirectangularSkyboxProcessor::genCubemapAndMipmaps(vk::Format format, glm::uvec2 size, std::span<const std::byte> data)
{
	cgpu::ImagePtr equirectangularImage = uploadEquirectangularImage(format, size, data);
	cgpu::ImagePtr cubemapImage = generateCubemap(format, equirectangularImage);
	generateMipmaps(cubemapImage);
	return downloadCubemapImage(cubemapImage);
}
