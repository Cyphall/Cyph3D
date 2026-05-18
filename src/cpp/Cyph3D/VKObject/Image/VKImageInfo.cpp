#include "VKImageInfo.h"

c3d::VKImageInfo::VKImageInfo(vk::Format format, const glm::uvec2& size, uint32_t layers, uint32_t levels, vk::ImageUsageFlags usage):
	_format(format),
	_size(size),
	_layers(layers),
	_levels(levels),
	_usage(usage)
{
	_compatibleViewFormats.push_back(format);
}

const vk::Format& c3d::VKImageInfo::getFormat() const
{
	return _format;
}

const glm::uvec2& c3d::VKImageInfo::getSize() const
{
	return _size;
}

const uint32_t& c3d::VKImageInfo::getLayers() const
{
	return _layers;
}

const uint32_t& c3d::VKImageInfo::getLevels() const
{
	return _levels;
}

const vk::ImageUsageFlags& c3d::VKImageInfo::getUsage() const
{
	return _usage;
}

void c3d::VKImageInfo::addRequiredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_requiredMemoryProperties |= property;
}

const vk::MemoryPropertyFlags& c3d::VKImageInfo::getRequiredMemoryProperties() const
{
	return _requiredMemoryProperties;
}

void c3d::VKImageInfo::addPreferredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_preferredMemoryProperties |= property;
}

const vk::MemoryPropertyFlags& c3d::VKImageInfo::getPreferredMemoryProperties() const
{
	return _preferredMemoryProperties;
}

void c3d::VKImageInfo::enableCubeCompatibility()
{
	_cubeCompatible = true;
}

const bool& c3d::VKImageInfo::isCubeCompatible() const
{
	return _cubeCompatible;
}

void c3d::VKImageInfo::addAdditionalCompatibleViewFormat(vk::Format format)
{
	_compatibleViewFormats.push_back(format);
}

const std::vector<vk::Format>& c3d::VKImageInfo::getCompatibleViewFormats() const
{
	return _compatibleViewFormats;
}

void c3d::VKImageInfo::setSwapchainImageHandle(vk::Image handle)
{
	_swapchainImageHandle = handle;
}

bool c3d::VKImageInfo::hasSwapchainImageHandle() const
{
	return _swapchainImageHandle.has_value();
}

const vk::Image& c3d::VKImageInfo::getSwapchainImageHandle() const
{
	return _swapchainImageHandle.value();
}

void c3d::VKImageInfo::setSampleCount(vk::SampleCountFlagBits sampleCount)
{
	_sampleCount = sampleCount;
}

const vk::SampleCountFlagBits& c3d::VKImageInfo::getSampleCount() const
{
	return _sampleCount;
}

void c3d::VKImageInfo::setName(std::string_view name)
{
	_name = name;
}

bool c3d::VKImageInfo::hasName() const
{
	return _name.has_value();
}

const std::string& c3d::VKImageInfo::getName() const
{
	return _name.value();
}

void c3d::VKImageInfo::setSwizzle(std::array<vk::ComponentSwizzle, 4> swizzle)
{
	_swizzle = swizzle;
}

bool c3d::VKImageInfo::hasSwizzle() const
{
	return _swizzle.has_value();
}

const std::array<vk::ComponentSwizzle, 4>& c3d::VKImageInfo::getSwizzle() const
{
	return _swizzle.value();
}