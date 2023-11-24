#include "VKImage.h"

#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKHelper.h"

#include <vulkan/vulkan_format_traits.hpp>
#include <set>

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
		vk::ImageCreateFlags flags = {};
		
		if (_info.getCompatibleViewFormats().size() > 1)
		{
			flags |= vk::ImageCreateFlagBits::eMutableFormat;
		}
		
		if (_info.isCubeCompatible())
		{
			flags |= vk::ImageCreateFlagBits::eCubeCompatible;
		}
		
		vk::ImageFormatListCreateInfo viewFormatsCreateInfo;
		viewFormatsCreateInfo.viewFormatCount = _info.getCompatibleViewFormats().size();
		viewFormatsCreateInfo.pViewFormats = _info.getCompatibleViewFormats().data();

		std::set<uint32_t> queues = {
			_context.getMainQueue().getFamily(),
			_context.getComputeQueue().getFamily(),
			_context.getTransferQueue().getFamily()
		};

		std::vector<uint32_t> queuesVec(queues.begin(), queues.end());
		
		vk::ImageCreateInfo createInfo;
		createInfo.imageType = vk::ImageType::e2D;
		createInfo.extent = vk::Extent3D(_info.getSize().x, _info.getSize().y, 1);
		createInfo.mipLevels = _info.getLevels();
		createInfo.arrayLayers = _info.getLayers();
		createInfo.format = _info.getFormat();
		createInfo.tiling = vk::ImageTiling::eOptimal;
		createInfo.initialLayout = vk::ImageLayout::eUndefined;
		createInfo.usage = _info.getUsage();
		createInfo.sharingMode = queuesVec.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = queuesVec.size();
		createInfo.pQueueFamilyIndices = queuesVec.data();
		createInfo.samples = _info.getSampleCount();
		createInfo.flags = flags;
		createInfo.pNext = &viewFormatsCreateInfo;
		
		vma::AllocationCreateInfo allocationCreateInfo;
		allocationCreateInfo.usage = vma::MemoryUsage::eUnknown;
		allocationCreateInfo.requiredFlags = _info.getRequiredMemoryProperties();
		allocationCreateInfo.preferredFlags = _info.getPreferredMemoryProperties();
		
		std::tie(_handle, _imageAlloc) = _context.getVmaAllocator().createImage(createInfo, allocationCreateInfo);
		
		if (_info.hasName())
		{
			vk::DebugUtilsObjectNameInfoEXT objectNameInfo;
			objectNameInfo.objectType = vk::ObjectType::eImage;
			objectNameInfo.objectHandle = reinterpret_cast<uintptr_t>(static_cast<VkImage>(_handle));
			objectNameInfo.pObjectName = _info.getName().c_str();
			
			_context.getDevice().setDebugUtilsObjectNameEXT(objectNameInfo);
		}
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
	
	for (auto& [viewInfo, view] : _views)
	{
		_context.getDevice().destroyImageView(view);
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

vk::ImageView VKImage::getView(vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format)
{
	ViewInfo viewInfo{
		.type = type,
		.layerRange = layerRange,
		.levelRange = levelRange,
		.format = format
	};
	
	auto it = _views.find(viewInfo);
	if (it == _views.end())
	{
		vk::ImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.image = _handle;
		imageViewCreateInfo.viewType = type;
		imageViewCreateInfo.format = format;
		if (_info.hasSwizzle())
		{
			imageViewCreateInfo.components.r = _info.getSwizzle()[0];
			imageViewCreateInfo.components.g = _info.getSwizzle()[1];
			imageViewCreateInfo.components.b = _info.getSwizzle()[2];
			imageViewCreateInfo.components.a = _info.getSwizzle()[3];
		}
		else
		{
			imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
			imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
		}
		imageViewCreateInfo.subresourceRange.aspectMask = VKHelper::getAspect(imageViewCreateInfo.format);
		imageViewCreateInfo.subresourceRange.baseArrayLayer = layerRange.x;
		imageViewCreateInfo.subresourceRange.layerCount = layerRange.y - layerRange.x + 1;
		imageViewCreateInfo.subresourceRange.baseMipLevel = levelRange.x;
		imageViewCreateInfo.subresourceRange.levelCount = levelRange.y - levelRange.x + 1;
		
		it = _views.try_emplace(viewInfo,  _context.getDevice().createImageView(imageViewCreateInfo)).first;
	}
	
	return it->second;
}

int VKImage::calcMaxMipLevels(const glm::uvec2& size)
{
	return static_cast<int>(glm::floor(glm::log2(static_cast<float>(glm::min(size.x, size.y))))) + 1;
}

void VKImage::setLayout(glm::uvec2 layerRange, glm::uvec2 levelRange, vk::ImageLayout layout)
{
	for (uint32_t layer = layerRange.x; layer <= layerRange.y; layer++)
	{
		for (uint32_t level = levelRange.x; level <= levelRange.y; level++)
		{
			_currentLayouts[layer][level] = layout;
		}
	}
}