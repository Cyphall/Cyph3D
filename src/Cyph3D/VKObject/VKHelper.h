#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <vulkan/vulkan.hpp>
#include <cstddef>

class VKContext;
class VKImageView;

class VKHelper
{
public:
	static size_t alignUp(size_t size, size_t alignment);
	
	static vk::ImageAspectFlags getAspect(vk::Format format);
	
	static void assertImageViewHasUniqueLayout(const VKPtr<VKImageView>& imageView);
};