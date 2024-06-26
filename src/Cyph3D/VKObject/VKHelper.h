#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

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

	static void assertImageViewHasUniqueState(const VKPtr<VKImage>& image, glm::uvec2 layerRange, glm::uvec2 levelRange);
};