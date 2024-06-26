#pragma once

#include <array>
#include <glm/glm.hpp>
#include <optional>
#include <vulkan/vulkan.hpp>

class VKImageInfo
{
public:
	explicit VKImageInfo(vk::Format format, const glm::uvec2& size, uint32_t layers, uint32_t levels, vk::ImageUsageFlags usage);

	const vk::Format& getFormat() const;

	const glm::uvec2& getSize() const;

	const uint32_t& getLayers() const;

	const uint32_t& getLevels() const;

	const vk::ImageUsageFlags& getUsage() const;

	void addRequiredMemoryProperty(vk::MemoryPropertyFlagBits property);
	const vk::MemoryPropertyFlags& getRequiredMemoryProperties() const;

	void addPreferredMemoryProperty(vk::MemoryPropertyFlagBits property);
	const vk::MemoryPropertyFlags& getPreferredMemoryProperties() const;

	void enableCubeCompatibility();
	const bool& isCubeCompatible() const;

	void addAdditionalCompatibleViewFormat(vk::Format format);
	const std::vector<vk::Format>& getCompatibleViewFormats() const;

	void setSwapchainImageHandle(vk::Image handle);
	bool hasSwapchainImageHandle() const;
	const vk::Image& getSwapchainImageHandle() const;

	void setSampleCount(vk::SampleCountFlagBits sampleCount);
	const vk::SampleCountFlagBits& getSampleCount() const;

	void setName(std::string_view name);
	bool hasName() const;
	const std::string& getName() const;

	void setSwizzle(std::array<vk::ComponentSwizzle, 4> swizzle);
	bool hasSwizzle() const;
	const std::array<vk::ComponentSwizzle, 4>& getSwizzle() const;

private:
	vk::Format _format;
	glm::uvec2 _size;
	uint32_t _layers;
	uint32_t _levels;
	vk::ImageUsageFlags _usage;
	vk::MemoryPropertyFlags _requiredMemoryProperties = {};
	vk::MemoryPropertyFlags _preferredMemoryProperties = {};
	bool _cubeCompatible = false;
	std::vector<vk::Format> _compatibleViewFormats;
	std::optional<vk::Image> _swapchainImageHandle;
	vk::SampleCountFlagBits _sampleCount = vk::SampleCountFlagBits::e1;
	std::optional<std::string> _name;
	std::optional<std::array<vk::ComponentSwizzle, 4>> _swizzle;
};