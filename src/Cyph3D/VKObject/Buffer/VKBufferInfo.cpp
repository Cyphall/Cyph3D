#include "VKBufferInfo.h"

VKBufferInfo::VKBufferInfo(size_t size, vk::BufferUsageFlags usage):
	_size(size),
	_usage(usage)
{
}

const size_t& VKBufferInfo::getSize() const
{
	return _size;
}

const vk::BufferUsageFlags& VKBufferInfo::getUsage() const
{
	return _usage;
}

void VKBufferInfo::addRequiredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_requiredMemoryProperties |= property;
}

void VKBufferInfo::setRequiredMemoryProperties(vk::MemoryPropertyFlags properties)
{
	_requiredMemoryProperties = properties;
}

const vk::MemoryPropertyFlags& VKBufferInfo::getRequiredMemoryProperties() const
{
	return _requiredMemoryProperties;
}

void VKBufferInfo::addPreferredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_preferredMemoryProperties |= property;
}

void VKBufferInfo::setPreferredMemoryProperties(vk::MemoryPropertyFlags properties)
{
	_preferredMemoryProperties = properties;
}

const vk::MemoryPropertyFlags& VKBufferInfo::getPreferredMemoryProperties() const
{
	return _preferredMemoryProperties;
}

void VKBufferInfo::setRequiredAlignment(vk::DeviceSize requiredAlignment)
{
	_requiredAlignment = requiredAlignment;
}

const vk::DeviceSize& VKBufferInfo::getRequiredAlignment() const
{
	return _requiredAlignment;
}

void VKBufferInfo::setName(std::string_view name)
{
	_name = name;
}

bool VKBufferInfo::hasName() const
{
	return _name.has_value();
}

const std::string& VKBufferInfo::getName() const
{
	return _name.value();
}