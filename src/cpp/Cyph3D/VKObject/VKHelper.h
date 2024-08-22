#pragma once


#include <cstddef>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKContext;
class VKImage;

class VKHelper
{
public:
	static size_t alignUp(size_t size, size_t alignment);

	static vk::ImageAspectFlags getAspect(vk::Format format);

	static void assertImageViewHasUniqueState(const std::shared_ptr<VKImage>& image, glm::uvec2 layerRange, glm::uvec2 levelRange);
};