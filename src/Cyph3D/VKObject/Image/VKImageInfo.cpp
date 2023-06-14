#include "VKImageInfo.h"

VKImageInfo::VKImageInfo(vk::Format format, const glm::uvec2& size, uint32_t layers, uint32_t levels, vk::ImageTiling tiling, vk::ImageUsageFlags usage):
	_format(format), _size(size), _layers(layers), _levels(levels), _tiling(tiling), _usage(usage)
{
	_compatibleViewFormats.push_back(format);
}

const vk::Format& VKImageInfo::getFormat() const
{
	return _format;
}

const glm::uvec2& VKImageInfo::getSize() const
{
	return _size;
}

const uint32_t& VKImageInfo::getLayers() const
{
	return _layers;
}

const uint32_t& VKImageInfo::getLevels() const
{
	return _levels;
}

const vk::ImageTiling& VKImageInfo::getTiling() const
{
	return _tiling;
}

const vk::ImageUsageFlags& VKImageInfo::getUsage() const
{
	return _usage;
}

void VKImageInfo::addRequiredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_requiredMemoryProperties |= property;
}

const vk::MemoryPropertyFlags& VKImageInfo::getRequiredMemoryProperties() const
{
	return _requiredMemoryProperties;
}

void VKImageInfo::addPreferredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_preferredMemoryProperties |= property;
}

const vk::MemoryPropertyFlags& VKImageInfo::getPreferredMemoryProperties() const
{
	return _preferredMemoryProperties;
}

void VKImageInfo::enableCubeCompatibility()
{
	_cubeCompatible = true;
}

const bool& VKImageInfo::isCubeCompatible() const
{
	return _cubeCompatible;
}

void VKImageInfo::addAdditionalCompatibleViewFormat(vk::Format format)
{
	_compatibleViewFormats.push_back(format);
}

const std::vector<vk::Format>& VKImageInfo::getCompatibleViewFormats() const
{
	return _compatibleViewFormats;
}

void VKImageInfo::setSwapchainImageHandle(vk::Image handle)
{
	_swapchainImageHandle = handle;
}

bool VKImageInfo::hasSwapchainImageHandle() const
{
	return _swapchainImageHandle.has_value();
}

const vk::Image& VKImageInfo::getSwapchainImageHandle() const
{
	return _swapchainImageHandle.value();
}

void VKImageInfo::setSampleCount(vk::SampleCountFlagBits sampleCount)
{
	_sampleCount = sampleCount;
}

const vk::SampleCountFlagBits& VKImageInfo::getSampleCount() const
{
	return _sampleCount;
}
