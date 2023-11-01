#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>

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
	
	void setRequiredAlignment(vk::DeviceSize requiredAlignment);
	const vk::DeviceSize& getRequiredAlignment() const;
	
	void setName(std::string_view name);
	bool hasName() const;
	const std::string& getName() const;

private:
	vk::BufferUsageFlags _usage;
	vk::MemoryPropertyFlags _requiredMemoryProperties = {};
	vk::MemoryPropertyFlags _preferredMemoryProperties = {};
	vk::DeviceSize _requiredAlignment = 1;
	std::optional<std::string> _name;
};