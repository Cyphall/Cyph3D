#include "EquirectangularSkyboxProcessor.h"

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
#include "Cyph3D/VKObject/Sampler/VKSampler.h"

#include <magic_enum.hpp>
#include <vulkan/vulkan_format_traits.hpp>
#include <array>
#include <filesystem>
#include <half.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct MipmapPushConstantData
{
	GLSL_bool srgb;
	GLSL_uint reduceMode;
};

struct CubemapPushConstantData
{
	GLSL_mat4 viewProjectionInv;
};

static void writeProcessedEquirectangularSkybox(const std::filesystem::path& path, const EquirectangularSkyboxData& equirectangularSkyboxData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file = FileHelper::openFileForWriting(path);
	
	uint8_t version = 1;
	FileHelper::write(file, &version);
	
	FileHelper::write(file, &equirectangularSkyboxData.format);
	
	FileHelper::write(file, &equirectangularSkyboxData.size);
	
	uint32_t levels = equirectangularSkyboxData.faces[0].size();
	FileHelper::write(file, &levels);
	
	for (uint32_t face = 0; face < equirectangularSkyboxData.faces.size(); face++)
	{
		for (uint32_t level = 0; level < equirectangularSkyboxData.faces[face].size(); level++)
		{
			FileHelper::write(file, equirectangularSkyboxData.faces[face][level]);
		}
	}
}

static bool readProcessedEquirectangularSkybox(const std::filesystem::path& path, EquirectangularSkyboxData& equirectangularSkyboxData)
{
	std::ifstream file = FileHelper::openFileForReading(path);
	
	uint8_t version;
	FileHelper::read(file, &version);
	
	if (version != 1)
	{
		return false;
	}
	
	FileHelper::read(file, &equirectangularSkyboxData.format);
	
	FileHelper::read(file, &equirectangularSkyboxData.size);
	
	uint32_t levels;
	FileHelper::read(file, &levels);
	
	for (uint32_t face = 0; face < equirectangularSkyboxData.faces.size(); face++)
	{
		equirectangularSkyboxData.faces[face].resize(levels);
		for (uint32_t level = 0; level < equirectangularSkyboxData.faces[face].size(); level++)
		{
			FileHelper::read(file, equirectangularSkyboxData.faces[face][level]);
		}
	}
	
	return true;
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

EquirectangularSkyboxData compressTexture(const EquirectangularSkyboxData& mipmappedEquirectangularSkyboxData, vk::Format requestedFormat)
{
	EquirectangularSkyboxData compressedEquirectangularSkyboxData;
	compressedEquirectangularSkyboxData.format = requestedFormat;
	compressedEquirectangularSkyboxData.size = mipmappedEquirectangularSkyboxData.size;
	
	for (uint32_t face = 0; face < mipmappedEquirectangularSkyboxData.faces.size(); face++)
	{
		glm::uvec2 size = mipmappedEquirectangularSkyboxData.size;
		for (uint32_t level = 0; level < mipmappedEquirectangularSkyboxData.faces[face].size(); level++)
		{
			std::vector<std::byte> compressedData;
			if (!ImageCompressor::tryCompressImage(mipmappedEquirectangularSkyboxData.faces[face][level], size, requestedFormat, compressedData))
				break;
			
			compressedEquirectangularSkyboxData.faces[face].emplace_back(std::move(compressedData));
			
			size = glm::max(size / 2u, glm::uvec2(1, 1));
		}
	}
	
	return compressedEquirectangularSkyboxData;
}

EquirectangularSkyboxProcessor::EquirectangularSkyboxProcessor()
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
			"resources/shaders/internal/asset processing/gen cubemap.comp");
		
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
			"resources/shaders/internal/asset processing/gen mipmap.comp");
		
		_mipmapPipeline = VKComputePipeline::create(Engine::getVKContext(), computePipelineInfo);
	}
}

EquirectangularSkyboxData EquirectangularSkyboxProcessor::readEquirectangularSkyboxData(AssetManagerWorkerData& workerData, std::string_view path, std::string_view cachePath)
{
	std::filesystem::path absolutePath = FileHelper::getAssetDirectoryPath() / path;
	std::filesystem::path cacheAbsolutePath = FileHelper::getCacheAssetDirectoryPath() / cachePath;
	
	EquirectangularSkyboxData equirectangularSkyboxData;
	
	if (std::filesystem::exists(cacheAbsolutePath))
	{
		Logger::info(std::format("Loading equirectangular skybox [{}] from cache...", path));
		if (readProcessedEquirectangularSkybox(cacheAbsolutePath, equirectangularSkyboxData))
		{
			Logger::info(std::format("Equirectangular skybox [{}] loaded from cache succesfully", path));
		}
		else
		{
			Logger::warning(std::format("Could not load equirectangular skybox [{}] from cache. Reprocessing...", path));
			std::filesystem::remove(cacheAbsolutePath);
			equirectangularSkyboxData = processEquirectangularSkybox(workerData, absolutePath, cacheAbsolutePath);
			Logger::info(std::format("Equirectangular skybox [{}] reprocessed succesfully", path));
		}
	}
	else
	{
		Logger::info(std::format("Processing equirectangular skybox [{}]", path));
		equirectangularSkyboxData = processEquirectangularSkybox(workerData, absolutePath, cacheAbsolutePath);
		Logger::info(std::format("Equirectangular skybox [{}] processed succesfully", path));
	}
	
	return equirectangularSkyboxData;
}

EquirectangularSkyboxData EquirectangularSkyboxProcessor::processEquirectangularSkybox(AssetManagerWorkerData& workerData, const std::filesystem::path& input, const std::filesystem::path& output)
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
	
	EquirectangularSkyboxData imageData = genCubemapAndMipmaps(workerData, cubemapAndMipmapGenFormat, image.getSize(), data, isMipmapGenFormatSrgb);
	
	if (compressionFormat != vk::Format::eUndefined)
	{
		imageData = compressTexture(imageData,	compressionFormat);
	}
	
	writeProcessedEquirectangularSkybox(output, imageData);
	
	return imageData;
}

static VKPtr<VKImage> uploadEquirectangularImage(AssetManagerWorkerData& workerData, vk::Format format, glm::uvec2 size, std::span<const std::byte> data)
{
	// create staging buffer
	VKBufferInfo stagingBufferInfo(data.size_bytes(), vk::BufferUsageFlagBits::eTransferSrc);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	
	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), stagingBufferInfo);
	
	// copy texture data to staging buffer
	std::memcpy(stagingBuffer->getHostPointer(), data.data(), stagingBufferInfo.getSize());
	
	// create equirectangular texture
	VKImageInfo imageInfo(
		format,
		size,
		1,
		1,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	VKPtr<VKImage> image = VKImage::create(Engine::getVKContext(), imageInfo);
	
	// upload staging buffer to texture
	workerData.transferCommandBuffer->begin();
	
	workerData.transferCommandBuffer->imageMemoryBarrier(
		image,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferWrite,
		vk::ImageLayout::eTransferDstOptimal);
	
	workerData.transferCommandBuffer->copyBufferToImage(stagingBuffer, 0, image, 0, 0);
	
	workerData.transferCommandBuffer->end();
	
	Engine::getVKContext().getTransferQueue().submit(workerData.transferCommandBuffer, {}, {});
	
	workerData.transferCommandBuffer->waitExecution();
	workerData.transferCommandBuffer->reset();
	
	return image;
}

VKPtr<VKImage> EquirectangularSkyboxProcessor::generateCubemap(AssetManagerWorkerData& workerData, vk::Format format, const VKPtr<VKImage>& equirectangularTexture)
{
	glm::uvec2 cubemapSize(equirectangularTexture->getInfo().getSize().y / 2);
	
	VKImageInfo cubemapTextureInfo(
		format,
		cubemapSize,
		6,
		VKImage::calcMaxMipLevels(cubemapSize),
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage);
	cubemapTextureInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	VKPtr<VKImage> cubemapTexture = VKImage::create(Engine::getVKContext(), cubemapTextureInfo);
	
	std::array<glm::mat4, 6> views = {
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 1,  0,  0), glm::vec3(0, 1,  0)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1,  0,  0), glm::vec3(0, 1,  0)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 0,  1,  0), glm::vec3(0, 0,  1)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 0, -1,  0), glm::vec3(0, 0, -1)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 0,  0, -1), glm::vec3(0, 1,  0)),
		glm::lookAt(glm::vec3(0, 0, 0), glm::vec3( 0,  0,  1), glm::vec3(0, 1,  0))
	};
	
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 10.0f);
	projection[1][1] *= -1;
	
	workerData.computeCommandBuffer->begin();
	
	workerData.computeCommandBuffer->imageMemoryBarrier(
		equirectangularTexture,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferWrite,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal);
	
	workerData.computeCommandBuffer->imageMemoryBarrier(
		cubemapTexture,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral);
	
	workerData.computeCommandBuffer->bindPipeline(_cubemapPipeline);
	
	workerData.computeCommandBuffer->pushDescriptor(0, 0, equirectangularTexture, _cubemapSampler);
	
	for (int i = 0; i < 6; i++)
	{
		workerData.computeCommandBuffer->pushDescriptor(
			0,
			1,
			cubemapTexture,
			vk::ImageViewType::e2D,
			{i, i},
			{0, 0},
			cubemapTexture->getInfo().getFormat());
		
		workerData.computeCommandBuffer->pushConstants(CubemapPushConstantData{
			.viewProjectionInv = glm::inverse(projection * views[i])
		});
		
		glm::uvec2 dstSize = cubemapTexture->getSize(0);
		workerData.computeCommandBuffer->dispatch({(dstSize.x + 7) / 8, (dstSize.y + 7) / 8, 1});
	}
	
	workerData.computeCommandBuffer->end();
	
	Engine::getVKContext().getComputeQueue().submit(workerData.computeCommandBuffer, {}, {});
	
	workerData.computeCommandBuffer->waitExecution();
	workerData.computeCommandBuffer->reset();
	
	return cubemapTexture;
}

void EquirectangularSkyboxProcessor::generateMipmaps(AssetManagerWorkerData& workerData, const VKPtr<VKImage>& cubemapTexture, bool isSrgb)
{
	workerData.computeCommandBuffer->begin();
	
	workerData.computeCommandBuffer->bindPipeline(_mipmapPipeline);
	
	workerData.computeCommandBuffer->pushConstants(MipmapPushConstantData{
		.srgb = isSrgb,
		.reduceMode = 0
	});
	
	workerData.computeCommandBuffer->imageMemoryBarrier(
		cubemapTexture,
		{0, 5},
		{0, 0},
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageRead,
		vk::ImageLayout::eGeneral);
	
	for (int level = 1; level < cubemapTexture->getInfo().getLevels(); level++)
	{
		workerData.computeCommandBuffer->imageMemoryBarrier(
			cubemapTexture,
			{0, 5},
			{level, level},
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageWrite,
			vk::ImageLayout::eGeneral);
		
		for (int face = 0; face < 6; face++)
		{
			workerData.computeCommandBuffer->pushDescriptor(
				0,
				0,
				cubemapTexture,
				vk::ImageViewType::e2D,
				{face, face},
				{level-1, level-1},
				cubemapTexture->getInfo().getFormat());
			
			workerData.computeCommandBuffer->pushDescriptor(
				0,
				1,
				cubemapTexture,
				vk::ImageViewType::e2D,
				{face, face},
				{level, level},
				cubemapTexture->getInfo().getFormat());
			
			glm::uvec2 dstSize = cubemapTexture->getSize(level);
			workerData.computeCommandBuffer->dispatch({(dstSize.x + 7) / 8, (dstSize.y + 7) / 8, 1});
		}
		
		workerData.computeCommandBuffer->imageMemoryBarrier(
			cubemapTexture,
			{0, 5},
			{level, level},
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageWrite,
			vk::PipelineStageFlagBits2::eComputeShader,
			vk::AccessFlagBits2::eShaderStorageRead,
			vk::ImageLayout::eGeneral);
	}
	
	workerData.computeCommandBuffer->imageMemoryBarrier(
		cubemapTexture,
		vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eShaderStorageRead,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferRead,
		vk::ImageLayout::eTransferSrcOptimal);
	
	workerData.computeCommandBuffer->end();
	
	Engine::getVKContext().getComputeQueue().submit(workerData.computeCommandBuffer, {}, {});
	
	workerData.computeCommandBuffer->waitExecution();
	workerData.computeCommandBuffer->reset();
}

static EquirectangularSkyboxData downloadCubemapTexture(AssetManagerWorkerData& workerData, const VKPtr<VKImage>& cubemapTexture)
{
	// create staging buffer
	VKBufferInfo stagingBufferInfo(cubemapTexture->getLayerByteSize() * 6, vk::BufferUsageFlagBits::eTransferDst);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	stagingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCached);
	
	VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), stagingBufferInfo);
	
	workerData.transferCommandBuffer->begin();
	
	vk::DeviceSize bufferOffset = 0;
	for (int face = 0; face < 6; face++)
	{
		for (int level = 0; level < cubemapTexture->getInfo().getLevels(); level++)
		{
			workerData.transferCommandBuffer->copyImageToBuffer(cubemapTexture, face, level, stagingBuffer, bufferOffset);
			bufferOffset += cubemapTexture->getLevelByteSize(level);
		}
	}
	
	workerData.transferCommandBuffer->end();
	
	Engine::getVKContext().getTransferQueue().submit(workerData.transferCommandBuffer, {}, {});
	
	workerData.transferCommandBuffer->waitExecution();
	workerData.transferCommandBuffer->reset();
	
	EquirectangularSkyboxData equirectangularSkyboxData;
	equirectangularSkyboxData.format = cubemapTexture->getInfo().getFormat();
	equirectangularSkyboxData.size = cubemapTexture->getInfo().getSize();
	
	std::byte* ptr = stagingBuffer->getHostPointer();
	for (int face = 0; face < 6; face++)
	{
		equirectangularSkyboxData.faces[face].resize(cubemapTexture->getInfo().getLevels());
		
		for (int level = 0; level < cubemapTexture->getInfo().getLevels(); level++)
		{
			equirectangularSkyboxData.faces[face][level].resize(cubemapTexture->getLevelByteSize(level));
			
			std::memcpy(equirectangularSkyboxData.faces[face][level].data(), ptr, cubemapTexture->getLevelByteSize(level));
			
			ptr += cubemapTexture->getLevelByteSize(level);
		}
	}
	
	return equirectangularSkyboxData;
}

EquirectangularSkyboxData EquirectangularSkyboxProcessor::genCubemapAndMipmaps(AssetManagerWorkerData& workerData, vk::Format format, glm::uvec2 size, std::span<const std::byte> data, bool isSrgb)
{
	VKPtr<VKImage> equirectangularTexture = uploadEquirectangularImage(workerData, format, size, data);
	VKPtr<VKImage> cubemapTexture = generateCubemap(workerData, format, equirectangularTexture);
	generateMipmaps(workerData, cubemapTexture, isSrgb);
	return downloadCubemapTexture(workerData, cubemapTexture);
}