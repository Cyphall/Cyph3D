#include "VKImage.h"

#include "Cyph3D/VKObject/VKContext.h"

#include <vulkan/vulkan_format_traits.hpp>

VKPtr<VKImage> VKImage::create(VKContext& context, const VKImageInfo& info)
{
	return VKPtr<VKImage>(new VKImage(context, info));
}

VKImage::VKImage(VKContext& context, const VKImageInfo& info):
	VKObject(context),
	_info(info)
{
	if (_info.hasSwapchainImageHandle())
	{
		_handle = _info.getSwapchainImageHandle();
	}
	else
	{
		vk::ImageCreateFlags flags = vk::ImageCreateFlagBits::eMutableFormat;
		
		if (_info.isCubeCompatible())
		{
			flags |= vk::ImageCreateFlagBits::eCubeCompatible;
		}
		
		vk::ImageFormatListCreateInfo viewFormatsCreateInfo;
		viewFormatsCreateInfo.viewFormatCount = _info.getCompatibleViewFormats().size();
		viewFormatsCreateInfo.pViewFormats = _info.getCompatibleViewFormats().data();
		
		vk::ImageCreateInfo createInfo;
		createInfo.imageType = vk::ImageType::e2D;
		createInfo.extent = vk::Extent3D(_info.getSize().x, _info.getSize().y, 1);
		createInfo.mipLevels = _info.getLevels();
		createInfo.arrayLayers = _info.getLayers();
		createInfo.format = _info.getFormat();
		createInfo.tiling = _info.getTiling();
		createInfo.initialLayout = vk::ImageLayout::eUndefined;
		createInfo.usage = _info.getUsage();
		createInfo.sharingMode = vk::SharingMode::eExclusive;
		createInfo.samples = _info.getSampleCount();
		createInfo.flags = flags;
		createInfo.pNext = &viewFormatsCreateInfo;
		
		vma::AllocationCreateInfo allocationCreateInfo;
		allocationCreateInfo.usage = vma::MemoryUsage::eUnknown;
		allocationCreateInfo.requiredFlags = _info.getRequiredMemoryProperties();
		allocationCreateInfo.preferredFlags = _info.getPreferredMemoryProperties();
		
		std::tie(_handle, _imageAlloc) = _context.getVmaAllocator().createImage(createInfo, allocationCreateInfo);
	}
	
	uint32_t layers = _info.getLayers();
	uint32_t levels = _info.getLevels();
	
	_currentLayouts.resize(layers);
	for (uint32_t i = 0; i < layers; i++)
	{
		_currentLayouts[i].resize(levels);
		std::fill(_currentLayouts[i].begin(), _currentLayouts[i].end(), vk::ImageLayout::eUndefined);
	}
	
	_sizes.resize(levels);
	_sizes[0] = _info.getSize();
	for (uint32_t i = 1; i < levels; i++)
	{
		_sizes[i] = glm::max(_sizes[i-1] / 2u, glm::uvec2(1, 1));
	}
}

VKImage::~VKImage()
{
	if (_imageAlloc)
	{
		_context.getVmaAllocator().destroyImage(_handle, _imageAlloc);
	}
}

const VKImageInfo& VKImage::getInfo() const
{
	return _info;
}

const vk::Image& VKImage::getHandle()
{
	return _handle;
}

const glm::uvec2& VKImage::getSize(uint32_t level) const
{
	return _sizes[level];
}

vk::ImageLayout VKImage::getLayout(uint32_t layer, uint32_t level) const
{
	return _currentLayouts[layer][level];
}

vk::DeviceSize VKImage::getLayerByteSize() const
{
	vk::DeviceSize totalSize = 0;
	for (uint32_t i = 0; i < _info.getLevels(); i++)
	{
		totalSize += getLevelByteSize(i);
	}
	return totalSize;
}

vk::DeviceSize VKImage::getLevelByteSize(uint32_t level) const
{
	vk::Format format = _info.getFormat();
	uint32_t blocksInXAxis = (_sizes[level].x + vk::blockExtent(format)[0] - 1) / vk::blockExtent(format)[0];
	uint32_t blocksInYAxis = (_sizes[level].y + vk::blockExtent(format)[1] - 1) / vk::blockExtent(format)[1];
	return blocksInXAxis * blocksInYAxis * vk::blockSize(format);
}

vk::DeviceSize VKImage::getPixelByteSize() const
{
	if (isCompressed())
	{
		throw;
	}
	
	vk::Format format = _info.getFormat();
	return vk::blockSize(format) / vk::texelsPerBlock(format);
}

bool VKImage::isCompressed() const
{
	return vk::isCompressed(_info.getFormat());
}

int VKImage::calcMaxMipLevels(const glm::uvec2& size)
{
	return static_cast<int>(glm::floor(glm::log2(static_cast<float>(glm::min(size.x, size.y))))) + 1;
}

void VKImage::setLayout(uint32_t layer, uint32_t level, vk::ImageLayout layout)
{
	_currentLayouts[layer][level] = layout;
}