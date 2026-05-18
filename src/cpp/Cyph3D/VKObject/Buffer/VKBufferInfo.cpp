#include "VKBufferInfo.h"

c3d::VKBufferInfo::VKBufferInfo(size_t size, vk::BufferUsageFlags usage):
	_size(size),
	_usage(usage)
{
}

const size_t& c3d::VKBufferInfo::getSize() const
{
	return _size;
}

const vk::BufferUsageFlags& c3d::VKBufferInfo::getUsage() const
{
	return _usage;
}

void c3d::VKBufferInfo::addRequiredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_requiredMemoryProperties |= property;
}

void c3d::VKBufferInfo::setRequiredMemoryProperties(vk::MemoryPropertyFlags properties)
{
	_requiredMemoryProperties = properties;
}

const vk::MemoryPropertyFlags& c3d::VKBufferInfo::getRequiredMemoryProperties() const
{
	return _requiredMemoryProperties;
}

void c3d::VKBufferInfo::addPreferredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_preferredMemoryProperties |= property;
}

void c3d::VKBufferInfo::setPreferredMemoryProperties(vk::MemoryPropertyFlags properties)
{
	_preferredMemoryProperties = properties;
}

const vk::MemoryPropertyFlags& c3d::VKBufferInfo::getPreferredMemoryProperties() const
{
	return _preferredMemoryProperties;
}

void c3d::VKBufferInfo::setRequiredAlignment(vk::DeviceSize requiredAlignment)
{
	_requiredAlignment = requiredAlignment;
}

const vk::DeviceSize& c3d::VKBufferInfo::getRequiredAlignment() const
{
	return _requiredAlignment;
}

void c3d::VKBufferInfo::setName(std::string_view name)
{
	_name = name;
}

bool c3d::VKBufferInfo::hasName() const
{
	return _name.has_value();
}

const std::string& c3d::VKBufferInfo::getName() const
{
	return _name.value();
}