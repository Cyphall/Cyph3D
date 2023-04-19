#pragma once

#include <vulkan/vulkan.hpp>
#include "Cyph3D/VKObject/VKObject.h"

class VKBufferBase : public VKObject
{
public:
	virtual const vk::Buffer& getHandle() = 0;
	
	virtual size_t getSize() const = 0;
	
	virtual vk::DeviceSize getByteSize() const = 0;
	
	virtual size_t getStride() const = 0;
	
protected:
	explicit VKBufferBase(VKContext& context):
		VKObject(context)
	{
		
	}
};