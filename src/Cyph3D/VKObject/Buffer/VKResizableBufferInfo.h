#pragma once

#include <vulkan/vulkan.hpp>

class VKResizableBufferInfo
{
public:
	explicit VKResizableBufferInfo(vk::BufferUsageFlags usage);
	
	const vk::BufferUsageFlags& getUsage() const;
	
	void addRequiredMemoryProperty(vk::MemoryPropertyFlagBits property);
	void setRequiredMemoryProperties(vk::MemoryPropertyFlags properties);
	const vk::MemoryPropertyFlags& getRequiredMemoryProperties() const;
	
	void addPreferredMemoryProperty(vk::MemoryPropertyFlagBits property);
	void setPreferredMemoryProperties(vk::MemoryPropertyFlags properties);
	const vk::MemoryPropertyFlags& getPreferredMemoryProperties() const;

private:
	vk::BufferUsageFlags _usage;
	vk::MemoryPropertyFlags _requiredMemoryProperties = {};
	vk::MemoryPropertyFlags _preferredMemoryProperties = {};
};