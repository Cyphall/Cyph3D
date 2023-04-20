#include "VKImage.h"

#include "Cyph3D/VKObject/VKContext.h"

#include <vulkan/vulkan_format_traits.hpp>

VKPtr<VKImage> VKImage::create(
	VKContext& context,
	vk::Format format,
	const glm::uvec2& size,
	uint32_t layers,
	uint32_t levels,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage,
	vk::ImageAspectFlagBits aspect,
	vk::MemoryPropertyFlags requiredProperties,
	vk::MemoryPropertyFlags preferredProperties,
	bool isCubeCompatible,
	const vk::ArrayProxy<vk::Format>& possibleViewFormats)
{
	return VKPtr<VKImage>(new VKImage(
		context,
		format,
		size,
		layers,
		levels,
		tiling,
		usage,
		aspect,
		requiredProperties,
		preferredProperties,
		isCubeCompatible,
		possibleViewFormats));
}

VKDynamic<VKImage> VKImage::createDynamic(
	VKContext& context,
	vk::Format format,
	const glm::uvec2& size,
	uint32_t layers,
	uint32_t levels,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage,
	vk::ImageAspectFlagBits aspect,
	vk::MemoryPropertyFlags requiredProperties,
	vk::MemoryPropertyFlags preferredProperties,
	bool isCubeCompatible,
	const vk::ArrayProxy<vk::Format>& possibleViewFormats)
{
	return VKDynamic<VKImage>(
		context,
		format,
		size,
		layers,
		levels,
		tiling,
		usage,
		aspect,
		requiredProperties,
		preferredProperties,
		isCubeCompatible,
		possibleViewFormats);
}

VKImage::VKImage(
	VKContext& context,
	vk::Format format,
	const glm::uvec2& size,
	uint32_t layers,
	uint32_t levels,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage,
	vk::ImageAspectFlagBits aspect,
	vk::MemoryPropertyFlags requiredProperties,
	vk::MemoryPropertyFlags preferredProperties,
	bool isCubeCompatible,
	const vk::ArrayProxy<vk::Format>& possibleViewFormats):
	VKObject(context),
	_ownsHandle(true),
	_format(format),
	_layers(layers),
	_levels(levels),
	_aspect(aspect)
{
	vk::ImageCreateFlags flags;
	
	if (isCubeCompatible)
	{
		flags |= vk::ImageCreateFlagBits::eCubeCompatible;
	}
	
	vk::ImageFormatListCreateInfo viewFormatsCreateInfo;
	if (!possibleViewFormats.empty())
	{
		flags |= vk::ImageCreateFlagBits::eMutableFormat;
		
		viewFormatsCreateInfo.viewFormatCount = possibleViewFormats.size();
		viewFormatsCreateInfo.pViewFormats = possibleViewFormats.data();
	}
	else
	{
		viewFormatsCreateInfo.viewFormatCount = 0;
		viewFormatsCreateInfo.pViewFormats = nullptr;
	}
	
	vk::ImageCreateInfo createInfo;
	createInfo.imageType = vk::ImageType::e2D;
	createInfo.extent = vk::Extent3D(size.x, size.y, 1);
	createInfo.mipLevels = levels;
	createInfo.arrayLayers = layers;
	createInfo.format = format;
	createInfo.tiling = tiling;
	createInfo.initialLayout = vk::ImageLayout::eUndefined;
	createInfo.usage = usage;
	createInfo.sharingMode = vk::SharingMode::eExclusive;
	createInfo.samples = vk::SampleCountFlagBits::e1;
	createInfo.flags = flags;
	createInfo.pNext = &viewFormatsCreateInfo;
	
	vma::AllocationCreateInfo allocationCreateInfo;
	allocationCreateInfo.usage = vma::MemoryUsage::eUnknown;
	allocationCreateInfo.requiredFlags = requiredProperties;
	allocationCreateInfo.preferredFlags = preferredProperties;
	
	std::tie(_handle, _imageAlloc) = _context.getVmaAllocator().createImage(createInfo, allocationCreateInfo);
	
	initLayoutsAndSizes(size);
}

VKImage::VKImage(
	VKContext& context,
	vk::Image handle,
	vk::Format format,
	const glm::uvec2& size,
	uint32_t layers,
	uint32_t levels,
	vk::ImageAspectFlagBits aspect):
	VKObject(context),
	_ownsHandle(false),
	_handle(handle),
	_format(format),
	_layers(layers),
	_levels(levels),
	_aspect(aspect)
{
	initLayoutsAndSizes(size);
}

VKImage::~VKImage()
{
	if (_ownsHandle)
	{
		_context.getVmaAllocator().destroyImage(_handle, _imageAlloc);
	}
}

const vk::Image& VKImage::getHandle()
{
	return _handle;
}

vk::Format VKImage::getFormat() const
{
	return _format;
}

const glm::uvec2& VKImage::getSize(uint32_t level) const
{
	return _sizes[level];
}

vk::ImageLayout VKImage::getLayout(uint32_t layer, uint32_t level) const
{
	return _currentLayouts[layer][level];
}

vk::ImageAspectFlags VKImage::getAspect() const
{
	return _aspect;
}

uint32_t VKImage::getLayers() const
{
	return _layers;
}

uint32_t VKImage::getLevels() const
{
	return _levels;
}

vk::DeviceSize VKImage::getLayerByteSize() const
{
	vk::DeviceSize totalSize = 0;
	for (uint32_t i = 0; i < getLevels(); i++)
	{
		totalSize += getLevelByteSize(i);
	}
	return totalSize;
}

vk::DeviceSize VKImage::getLevelByteSize(uint32_t level) const
{
	uint32_t blocksInXAxis = (_sizes[level].x + vk::blockExtent(_format)[0] - 1) / vk::blockExtent(_format)[0];
	uint32_t blocksInYAxis = (_sizes[level].y + vk::blockExtent(_format)[1] - 1) / vk::blockExtent(_format)[1];
	return blocksInXAxis * blocksInYAxis * vk::blockSize(_format);
}

vk::DeviceSize VKImage::getPixelByteSize() const
{
	if (isCompressed())
	{
		throw;
	}
	
	return vk::blockSize(_format) / vk::texelsPerBlock(_format);
}

bool VKImage::isCompressed() const
{
	return vk::isCompressed(_format);
}

int VKImage::calcMaxMipLevels(const glm::uvec2& size)
{
	return static_cast<int>(glm::floor(glm::log2(static_cast<float>(glm::max(size.x, size.y))))) + 1;
}

void VKImage::initLayoutsAndSizes(glm::uvec2 size)
{
	_currentLayouts.resize(_layers);
	for (uint32_t i = 0; i < _layers; i++)
	{
		_currentLayouts[i].resize(_levels);
		std::fill(_currentLayouts[i].begin(), _currentLayouts[i].end(), vk::ImageLayout::eUndefined);
	}
	
	_sizes.resize(_levels);
	_sizes[0] = size;
	for (uint32_t i = 1; i < _levels; i++)
	{
		_sizes[i] = glm::max(_sizes[i-1] / 2u, glm::uvec2(1, 1));
	}
}

void VKImage::setLayout(vk::ImageLayout layout, uint32_t layer, uint32_t level)
{
	_currentLayouts[layer][level] = layout;
}