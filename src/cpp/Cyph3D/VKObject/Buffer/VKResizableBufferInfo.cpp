#include "VKResizableBufferInfo.h"

c3d::VKResizableBufferInfo::VKResizableBufferInfo(vk::BufferUsageFlags usage):
	_usage(usage)
{
}

const vk::BufferUsageFlags& c3d::VKResizableBufferInfo::getUsage() const
{
	return _usage;
}

void c3d::VKResizableBufferInfo::addRequiredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_requiredMemoryProperties |= property;
}

void c3d::VKResizableBufferInfo::setRequiredMemoryProperties(vk::MemoryPropertyFlags properties)
{
	_requiredMemoryProperties = properties;
}

const vk::MemoryPropertyFlags& c3d::VKResizableBufferInfo::getRequiredMemoryProperties() const
{
	return _requiredMemoryProperties;
}

void c3d::VKResizableBufferInfo::addPreferredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_preferredMemoryProperties |= property;
}

void c3d::VKResizableBufferInfo::setPreferredMemoryProperties(vk::MemoryPropertyFlags properties)
{
	_preferredMemoryProperties = properties;
}

const vk::MemoryPropertyFlags& c3d::VKResizableBufferInfo::getPreferredMemoryProperties() const
{
	return _preferredMemoryProperties;
}

void c3d::VKResizableBufferInfo::setRequiredAlignment(vk::DeviceSize requiredAlignment)
{
	_requiredAlignment = requiredAlignment;
}

const vk::DeviceSize& c3d::VKResizableBufferInfo::getRequiredAlignment() const
{
	return _requiredAlignment;
}

void c3d::VKResizableBufferInfo::setName(std::string_view name)
{
	_name = name;
}

bool c3d::VKResizableBufferInfo::hasName() const
{
	return _name.has_value();
}

const std::string& c3d::VKResizableBufferInfo::getName() const
{
	return _name.value();
}