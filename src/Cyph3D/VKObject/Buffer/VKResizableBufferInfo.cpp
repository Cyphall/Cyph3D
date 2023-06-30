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