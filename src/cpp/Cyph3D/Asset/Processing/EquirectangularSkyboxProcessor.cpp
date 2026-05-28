#include "EquirectangularSkyboxProcessor.h"

#include <Cyph3D/Asset/AssetManagerWorkerData.h>
#include <Cyph3D/Asset/Processing/ImageCompressor.h>
#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/StbImage.h>
#include <Cyph3D/VKObject/Buffer/VKBuffer.h>
#include <Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h>
#include <Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h>
#include <Cyph3D/VKObject/Image/VKImage.h>
#include <Cyph3D/VKObject/Pipeline/VKComputePipeline.h>
#include <Cyph3D/VKObject/Pipeline/VKPipelineLayout.h>
#include <Cyph3D/VKObject/Queue/VKQueue.h>
#include <Cyph3D/VKObject/Sampler/VKSampler.h>

#include <array>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <half.hpp>
#include <spdlog/spdlog.h>

namespace
{
struct MipmapPushConstantData
{
	vk::Bool32 srgb;
	uint32_t reduceMode;
};

struct CubemapPushConstantData
{
	glm::mat4 viewProjectionInv;
};

void writeProcessedEquirectangularSkybox(const std::filesystem::path& path, const c3d::EquirectangularSkyboxData& equirectangularSkyboxData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file = c3d::FileHelper::openFileForWriting(path);

	uint8_t version = 1;
	c3d::FileHelper::write(file, &version);

	c3d::FileHelper::write(file, &equirectangularSkyboxData.format);

	c3d::FileHelper::write(file, &equirectangularSkyboxData.size);

	uint32_t levels = equirectangularSkyboxData.faces[0].size();
	c3d::FileHelper::write(file, &levels);

	for (uint32_t face = 0; face < equirectangularSkyboxData.faces.size(); face++)
	{
		for (uint32_t level = 0; level < equirectangularSkyboxData.faces[face].size(); level++)
		{
			c3d::FileHelper::write(file, equirectangularSkyboxData.faces[face][level]);
		}
	}
}

bool readProcessedEquirectangularSkybox(const std::filesystem::path& path, c3d::EquirectangularSkyboxData& equirectangularSkyboxData)
{
	std::ifstream file = c3d::FileHelper::openFileForReading(path);

	uint8_t version;
	c3d::FileHelper::read(file, &version);

	if (version != 1)
	{
		return false;
	}

	c3d::FileHelper::read(file, &equirectangularSkyboxData.format);

	c3d::FileHelper::read(file, &equirectangularSkyboxData.size);

	uint32_t levels;
	c3d::FileHelper::read(file, &levels);

	for (uint32_t face = 0; face < equirectangularSkyboxData.faces.size(); face++)
	{
		equirectangularSkyboxData.faces[face].resize(levels);
		for (uint32_t level = 0; level < equirectangularSkyboxData.faces[face].size(); level++)
		{
			c3d::FileHelper::read(file, equirectangularSkyboxData.faces[face][level]);
		}
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

c3d::EquirectangularSkyboxData compressTexture(const c3d::EquirectangularSkyboxData& mipmappedEquirectangularSkyboxData, vk::Format requestedFormat)
{
	c3d::EquirectangularSkyboxData compressedEquirectangularSkyboxData;
	compressedEquirectangularSkyboxData.format = requestedFormat;
	compressedEquirectangularSkyboxData.size = mipmappedEquirectangularSkyboxData.size;

	for (uint32_t face = 0; face < mipmappedEquirectangularSkyboxData.faces.size(); face++)
	{
		glm::uvec2 size = mipmappedEquirectangularSkyboxData.size;
		for (uint32_t level = 0; level < mipmappedEquirectangularSkyboxData.faces[face].size(); level++)
		{
			std::vector<std::byte> compressedData;
			if (!c3d::ImageCompressor::tryCompressImage(mipmappedEquirectangularSkyboxData.faces[face][level], size, requestedFormat, compressedData))
				break;

			compressedEquirectangularSkyboxData.faces[face].emplace_back(std::move(compressedData));

			size = glm::max(size / 2u, glm::uvec2(1, 1));
		}
	}

	return compressedEquirectangularSkyboxData;
}

std::shared_ptr<c3d::VKImage> uploadEquirectangularImage(vk::Format format, glm::uvec2 size, std::span<const std::byte> data)
{
	// create staging buffer
	c3d::VKBufferInfo stagingBufferInfo(data.size_bytes(), vk::BufferUsageFlagBits::eTransferSrc);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);

	std::shared_ptr<c3d::VKBuffer<std::byte>> stagingBuffer = c3d::VKBuffer<std::byte>::create(c3d::Engine::getVKContext(), stagingBufferInfo);

	// copy texture data to staging buffer
	std::copy_n(data.data(), data.size(), stagingBuffer->getHostPointer());

	// create equirectangular texture
	c3d::VKImageInfo imageInfo(
		format,
		size,
		1,
		1,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);

	std::shared_ptr<c3d::VKImage> image = c3d::VKImage::create(c3d::Engine::getVKContext(), imageInfo);

	// upload staging buffer to texture
	c3d::assetTransferCommandBuffer->begin();

	c3d::assetTransferCommandBuffer->bufferMemoryBarrier(
		stagingBuffer,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferRead
	);

	c3d::assetTransferCommandBuffer->imageMemoryBarrier(
		image,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferWrite,
		vk::ImageLayout::eTransferDstOptimal
	);

	c3d::assetTransferCommandBuffer->copyBufferToImage(stagingBuffer, 0, image, 0, 0);

	c3d::assetTransferCommandBuffer->releaseImageOwnership(
		image,
		c3d::Engine::getVKContext().getComputeQueue(),
		vk::ImageLayout::eReadOnlyOptimal
	);

	c3d::assetTransferCommandBuffer->end();

	c3d::Engine::getVKContext().getTransferQueue().submit(c3d::assetTransferCommandBuffer, {}, {});

	c3d::assetTransferCommandBuffer->waitExecution();
	c3d::assetTransferCommandBuffer->reset();

	return image;
}

c3d::EquirectangularSkyboxData downloadCubemapTexture(const std::shared_ptr<c3d::VKImage>& cubemapTexture)
{
	// create staging buffer
	c3d::VKBufferInfo stagingBufferInfo(cubemapTexture->getLayerByteSize() * 6, vk::BufferUsageFlagBits::eTransferDst);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCached);

	std::shared_ptr<c3d::VKBuffer<std::byte>> stagingBuffer = c3d::VKBuffer<std::byte>::create(c3d::Engine::getVKContext(), stagingBufferInfo);

	c3d::assetTransferCommandBuffer->begin();

	c3d::assetTransferCommandBuffer->acquireImageOwnership(
		cubemapTexture,
		c3d::Engine::getVKContext().getComputeQueue(),
		vk::PipelineStageFlagBits2::eTransfer,
		vk::AccessFlagBits2::eTransferRead,
		vk::ImageLayout::eTransferSrcOptimal
	);

	c3d::assetTransferCommandBuffer->bufferMemoryBarrier(
		stagingBuffer,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferWrite
	);

	vk::DeviceSize bufferOffset = 0;
	for (int face = 0; face < 6; face++)
	{
		for (int level = 0; level < cubemapTexture->getInfo().getLevels(); level++)
		{
			c3d::assetTransferCommandBuffer->copyImageToBuffer(cubemapTexture, face, level, stagingBuffer, bufferOffset);
			bufferOffset += cubemapTexture->getLevelByteSize(level);
		}
	}

	c3d::assetTransferCommandBuffer->end();

	c3d::Engine::getVKContext().getTransferQueue().submit(c3d::assetTransferCommandBuffer, {}, {});

	c3d::assetTransferCommandBuffer->waitExecution();
	c3d::assetTransferCommandBuffer->reset();

	c3d::EquirectangularSkyboxData equirectangularSkyboxData;
	equirectangularSkyboxData.format = cubemapTexture->getInfo().getFormat();
	equirectangularSkyboxData.size = cubemapTexture->getInfo().getSize();

	std::byte* ptr = stagingBuffer->getHostPointer();
	for (int face = 0; face < 6; face++)
	{
		equirectangularSkyboxData.faces[face].resize(cubemapTexture->getInfo().getLevels());

		for (int level = 0; level < cubemapTexture->getInfo().getLevels(); level++)
		{
			equirectangularSkyboxData.faces[face][level].resize(cubemapTexture->getLevelByteSize(level));

			std::copy_n(ptr, equirectangularSkyboxData.faces[face][level].size(), equirectangularSkyboxData.faces[face][level].data());
			ptr += equirectangularSkyboxData.faces[face][level].size();
		}
	}

	return equirectangularSkyboxData;
}
}

c3d::EquirectangularSkyboxProcessor::EquirectangularSkyboxProcessor()
{
	{
		VKDescriptorSetLayoutInfo descriptorSetLayoutInfo(true);
		descriptorSetLayoutInfo.addBinding(vk::DescriptorType::eCombinedImageSampler, 1);
		descriptorSetLayoutInfo.addBinding(vk::DescriptorType::eStorageImage, 1);

		_cubemapDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), descriptorSetLayoutInfo);

		VKPipelineLayoutInfo pipelineLayoutInfo;
		pipelineLayoutInfo.addDescriptorSetLayout(_cubemapDescriptorSetLayout);
		pipelineLayoutInfo.setPushConstantLayout<CubemapPushConstantData>();

		_cubemapPipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), pipelineLayoutInfo);

		VKComputePipelineInfo computePipelineInfo(
			_cubemapPipelineLayout,
			"asset processing/gen cubemap.comp"
		);

		_cubemapPipeline = VKComputePipeline::create(Engine::getVKContext(), computePipelineInfo);

		vk::SamplerCreateInfo samplerCreateInfo;
		samplerCreateInfo.flags = {};
		samplerCreateInfo.magFilter = vk::Filter::eLinear;
		samplerCreateInfo.minFilter = vk::Filter::eLinear;
		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.anisotropyEnable = false;
		samplerCreateInfo.maxAnisotropy = 1;
		samplerCreateInfo.compareEnable = false;
		samplerCreateInfo.compareOp = vk::CompareOp::eNever;
		samplerCreateInfo.minLod = -1000.0f;
		samplerCreateInfo.maxLod = 1000.0f;
		samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerCreateInfo.unnormalizedCoordinates = false;

		_cubemapSampler = VKSampler::create(Engine::getVKContext(), samplerCreateInfo);
	}

	{
		VKDescriptorSetLayoutInfo descriptorSetLayoutInfo(true);
		descriptorSetLayoutInfo.addBinding(vk::DescriptorType::eStorageImage, 1);
		descriptorSetLayoutInfo.addBinding(vk::DescriptorType::eStorageImage, 1);

		_mipmapDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), descriptorSetLayoutInfo);

		VKPipelineLayoutInfo pipelineLayoutInfo;
		pipelineLayoutInfo.addDescriptorSetLayout(_mipmapDescriptorSetLayout);
		pipelineLayoutInfo.setPushConstantLayout<MipmapPushConstantData>();

		_mipmapPipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), pipelineLayoutInfo);

		VKComputePipelineInfo computePipelineInfo(
			_mipmapPipelineLayout,
			"asset processing/gen mipmap.comp"
		);

		_mipmapPipeline = VKComputePipeline::create(Engine::getVKContext(), computePipelineInfo);
	}
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
	bool isMipmapGenFormatSrgb;
	vk::Format compressionFormat;
	switch (image.getBitsPerChannel())
	{
	case 8:
		cubemapAndMipmapGenFormat = vk::Format::eR8G8B8A8Unorm;
		isMipmapGenFormatSrgb = true;
		compressionFormat = vk::Format::eBc7SrgbBlock;
		break;
	case 32:
		cubemapAndMipmapGenFormat = vk::Format::eR16G16B16A16Sfloat;
		isMipmapGenFormatSrgb = false;
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

	EquirectangularSkyboxData imageData = genCubemapAndMipmaps(cubemapAndMipmapGenFormat, image.getSize(), data, isMipmapGenFormatSrgb);

	if (compressionFormat != vk::Format::eUndefined)
	{
		imageData = compressTexture(imageData, compressionFormat);
	}

	writeProcessedEquirectangularSkybox(output, imageData);

	return imageData;
}

std::shared_ptr<c3d::VKImage> c3d::EquirectangularSkyboxProcessor::generateCubemap(vk::Format format, const std::shared_ptr<VKImage>& equirectangularTexture)
{
	glm::uvec2 cubemapSize(equirectangularTexture->getInfo().getSize().y / 2);

	VKImageInfo cubemapTextureInfo(
		format,
		cubemapSize,
		6,
		VKImage::calcMaxMipLevels(cubemapSize),
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage
	);
	cubemapTextureInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);

	std::shared_ptr<VKImage> cubemapTexture = VKImage::create(Engine::getVKContext(), cubemapTextureInfo);

	std::array<glm::mat4, 6> views = {
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0))
	};

	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 10.0f);
	projection[1][1] *= -1;

	assetComputeCommandBuffer->begin();

	assetComputeCommandBuffer->acquireImageOwnership(
		equirectangularTexture,
		Engine::getVKContext().getTransferQueue(),
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);

	assetComputeCommandBuffer->imageMemoryBarrier(
		cubemapTexture,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral,
		{0, 5},
		{0, 0}
	);

	assetComputeCommandBuffer->bindPipeline(_cubemapPipeline);

	assetComputeCommandBuffer->pushDescriptor(0, 0, equirectangularTexture, _cubemapSampler);

	for (int i = 0; i < 6; i++)
	{
		assetComputeCommandBuffer->pushDescriptor(
			0,
			1,
			cubemapTexture,
			vk::ImageViewType::e2D,
			{i, i},
			{0, 0},
			cubemapTexture->getInfo().getFormat()
		);

		assetComputeCommandBuffer->pushConstants(CubemapPushConstantData{.viewProjectionInv = glm::inverse(projection * views[i])});

		glm::uvec2 dstSize = cubemapTexture->getSize(0);
		assetComputeCommandBuffer->dispatch({(dstSize.x + 7) / 8, (dstSize.y + 7) / 8, 1});
	}

	assetComputeCommandBuffer->end();

	Engine::getVKContext().getComputeQueue().submit(assetComputeCommandBuffer, {}, {});

	assetComputeCommandBuffer->waitExecution();
	assetComputeCommandBuffer->reset();

	return cubemapTexture;
}

void c3d::EquirectangularSkyboxProcessor::generateMipmaps(const std::shared_ptr<VKImage>& cubemapTexture, bool isSrgb)
{
	assetComputeCommandBuffer->begin();

	assetComputeCommandBuffer->bindPipeline(_mipmapPipeline);

	assetComputeCommandBuffer->pushConstants(MipmapPushConstantData{.srgb = isSrgb, .reduceMode = 0});

	assetComputeCommandBuffer->imageMemoryBarrier(
		cubemapTexture,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageRead,
		vk::ImageLayout::eGeneral,
		{0, 5},
		{0, 0}
	);

	for (int level = 1; level < cubemapTexture->getInfo().getLevels(); level++)
	{
		assetComputeCommandBuffer->imageMemoryBarrier(
			cubemapTexture,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageWrite,
			vk::ImageLayout::eGeneral,
			{0, 5},
			{level, level}
		);

		for (int face = 0; face < 6; face++)
		{
			assetComputeCommandBuffer->pushDescriptor(
				0,
				0,
				cubemapTexture,
				vk::ImageViewType::e2D,
				{face, face},
				{level - 1, level - 1},
				cubemapTexture->getInfo().getFormat()
			);

			assetComputeCommandBuffer->pushDescriptor(
				0,
				1,
				cubemapTexture,
				vk::ImageViewType::e2D,
				{face, face},
				{level, level},
				cubemapTexture->getInfo().getFormat()
			);

			glm::uvec2 dstSize = cubemapTexture->getSize(level);
			assetComputeCommandBuffer->dispatch({(dstSize.x + 7) / 8, (dstSize.y + 7) / 8, 1});
		}

		assetComputeCommandBuffer->imageMemoryBarrier(
			cubemapTexture,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageRead,
			vk::ImageLayout::eGeneral,
			{0, 5},
			{level, level}
		);
	}

	assetComputeCommandBuffer->releaseImageOwnership(
		cubemapTexture,
		Engine::getVKContext().getTransferQueue(),
		vk::ImageLayout::eTransferSrcOptimal
	);

	assetComputeCommandBuffer->end();

	Engine::getVKContext().getComputeQueue().submit(assetComputeCommandBuffer, {}, {});

	assetComputeCommandBuffer->waitExecution();
	assetComputeCommandBuffer->reset();
}

c3d::EquirectangularSkyboxData c3d::EquirectangularSkyboxProcessor::genCubemapAndMipmaps(vk::Format format, glm::uvec2 size, std::span<const std::byte> data, bool isSrgb)
{
	std::shared_ptr<VKImage> equirectangularTexture = uploadEquirectangularImage(format, size, data);
	std::shared_ptr<VKImage> cubemapTexture = generateCubemap(format, equirectangularTexture);
	generateMipmaps(cubemapTexture, isSrgb);
	return downloadCubemapTexture(cubemapTexture);
}