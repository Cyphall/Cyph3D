#include "VKImage.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKHelper.h"

#include <ranges>
#include <vulkan/vulkan_format_traits.hpp>

std::shared_ptr<VKImage> VKImage::create(VKContext& context, const VKImageInfo& info)
{
	return std::shared_ptr<VKImage>(new VKImage(context, info));
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

		vk::ImageCreateInfo imageCreateInfo;
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.extent = vk::Extent3D(_info.getSize().x, _info.getSize().y, 1);
		imageCreateInfo.mipLevels = _info.getLevels();
		imageCreateInfo.arrayLayers = _info.getLayers();
		imageCreateInfo.format = _info.getFormat();
		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.usage = _info.getUsage();
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.samples = _info.getSampleCount();
		imageCreateInfo.flags = flags;
		imageCreateInfo.pNext = &viewFormatsCreateInfo;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_UNKNOWN;
		allocationCreateInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(_info.getRequiredMemoryProperties());
		allocationCreateInfo.preferredFlags = static_cast<VkMemoryPropertyFlags>(_info.getPreferredMemoryProperties());
		allocationCreateInfo.memoryTypeBits = std::numeric_limits<uint32_t>::max();
		allocationCreateInfo.pool = nullptr;
		allocationCreateInfo.pUserData = nullptr;
		allocationCreateInfo.priority = 0.5f;

		vmaCreateImage(
			_context.getVmaAllocator(),
			reinterpret_cast<VkImageCreateInfo*>(&imageCreateInfo),
			&allocationCreateInfo,
			reinterpret_cast<VkImage*>(&_handle),
			&_allocation,
			nullptr
		);
	}

	if (_info.hasName())
	{
		vk::DebugUtilsObjectNameInfoEXT objectNameInfo;
		objectNameInfo.objectType = vk::ObjectType::eImage;
		objectNameInfo.objectHandle = reinterpret_cast<uintptr_t>(static_cast<VkImage>(_handle));
		objectNameInfo.pObjectName = _info.getName().c_str();

		_context.getDevice().setDebugUtilsObjectNameEXT(objectNameInfo);
	}

	uint32_t layers = _info.getLayers();
	uint32_t levels = _info.getLevels();

	_currentStates.resize(layers);
	for (uint32_t i = 0; i < layers; i++)
	{
		_currentStates[i].resize(levels);
		std::ranges::fill(
			_currentStates[i],
			State{
				.layout = vk::ImageLayout::eUndefined,
				.stageMask = _info.hasSwapchainImageHandle() ? vk::PipelineStageFlagBits2::eColorAttachmentOutput : vk::PipelineStageFlagBits2::eNone,
				.accessMask = vk::AccessFlagBits2::eNone,
			}
		);
	}

	_sizes.resize(levels);
	_sizes[0] = _info.getSize();
	for (uint32_t i = 1; i < levels; i++)
	{
		_sizes[i] = glm::max(_sizes[i - 1] / 2u, glm::uvec2(1, 1));
	}
}

VKImage::~VKImage()
{
	for (vk::ImageView view : _views | std::views::values)
	{
		_context.getDevice().destroyImageView(view);
	}

	if (_allocation)
	{
		vmaDestroyImage(_context.getVmaAllocator(), _handle, _allocation);
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

const VKImage::State& VKImage::getState(uint32_t layer, uint32_t level) const
{
	return _currentStates[layer][level];
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

		it = _views.try_emplace(viewInfo, _context.getDevice().createImageView(imageViewCreateInfo)).first;

		if (_info.hasName())
		{
			vk::DebugUtilsObjectNameInfoEXT objectNameInfo;
			objectNameInfo.objectType = vk::ObjectType::eImageView;
			objectNameInfo.objectHandle = reinterpret_cast<uintptr_t>(static_cast<VkImageView>(it->second));
			objectNameInfo.pObjectName = _info.getName().c_str();

			_context.getDevice().setDebugUtilsObjectNameEXT(objectNameInfo);
		}
	}

	return it->second;
}

int VKImage::calcMaxMipLevels(const glm::uvec2& size)
{
	return static_cast<int>(glm::floor(glm::log2(static_cast<float>(glm::min(size.x, size.y))))) + 1;
}

void VKImage::setState(glm::uvec2 layerRange, glm::uvec2 levelRange, const State& state)
{
	for (uint32_t layer = layerRange.x; layer <= layerRange.y; layer++)
	{
		for (uint32_t level = levelRange.x; level <= levelRange.y; level++)
		{
			_currentStates[layer][level] = state;
		}
	}
}