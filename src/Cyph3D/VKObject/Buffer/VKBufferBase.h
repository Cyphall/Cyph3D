#pragma once

#include "Cyph3D/VKObject/Buffer/VKBufferInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKBufferBase : public VKObject
{
public:
	const VKBufferInfo& getInfo() const
	{
		return _info;
	}

	virtual const vk::Buffer& getHandle() = 0;

	vk::DeviceSize getByteSize() const
	{
		return _info.getSize() * getStride();
	}

	virtual size_t getStride() const = 0;

	virtual vk::DeviceAddress getDeviceAddress() const = 0;

protected:
	VKBufferInfo _info;

	explicit VKBufferBase(VKContext& context, const VKBufferInfo& info):
		VKObject(context),
		_info(info)
	{
	}
};