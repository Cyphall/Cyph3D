#include "VKResizableBufferInfo.h"

VKResizableBufferInfo::VKResizableBufferInfo(vk::BufferUsageFlags usage):
	_usage(usage)
{
	
}

const vk::BufferUsageFlags& VKResizableBufferInfo::getUsage() const
{
	return _usage;
}

void VKResizableBufferInfo::addRequiredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_requiredMemoryProperties |= property;
}

void VKResizableBufferInfo::setRequiredMemoryProperties(vk::MemoryPropertyFlags properties)
{
	_requiredMemoryProperties = properties;
}

const vk::MemoryPropertyFlags& VKResizableBufferInfo::getRequiredMemoryProperties() const
{
	return _requiredMemoryProperties;
}

void VKResizableBufferInfo::addPreferredMemoryProperty(vk::MemoryPropertyFlagBits property)
{
	_preferredMemoryProperties |= property;
}

void VKResizableBufferInfo::setPreferredMemoryProperties(vk::MemoryPropertyFlags properties)
{
	_preferredMemoryProperties = properties;
}

const vk::MemoryPropertyFlags& VKResizableBufferInfo::getPreferredMemoryProperties() const
{
	return _preferredMemoryProperties;
}

void VKResizableBufferInfo::setRequiredAlignment(vk::DeviceSize requiredAlignment)
{
	_requiredAlignment = requiredAlignment;
}

const vk::DeviceSize& VKResizableBufferInfo::getRequiredAlignment() const
{
	return _requiredAlignment;
}

void VKResizableBufferInfo::setName(std::string_view name)
{
	_name = name;
}

bool VKResizableBufferInfo::hasName() const
{
	return _name.has_value();
}

const std::string& VKResizableBufferInfo::getName() const
{
	return _name.value();
}