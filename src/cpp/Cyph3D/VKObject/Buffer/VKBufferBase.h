#pragma once

#include "Cyph3D/VKObject/Buffer/VKBufferInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKBufferBase : public VKObject
{
public:
	struct State
	{
		vk::PipelineStageFlags2 stageMask;
		vk::AccessFlags2 accessMask;

		bool operator==(const State& other) const = default;
	};

	const VKBufferInfo& getInfo() const;

	virtual const vk::Buffer& getHandle() = 0;

	vk::DeviceSize getByteSize() const;

	virtual size_t getStride() const = 0;

	virtual vk::DeviceAddress getDeviceAddress() const = 0;

	const State& getState() const;

protected:
	friend class VKCommandBuffer;

	VKBufferInfo _info;

	State _state = {
		.stageMask = vk::PipelineStageFlagBits2::eNone,
		.accessMask = vk::AccessFlagBits2::eNone,
	};

	explicit VKBufferBase(VKContext& context, const VKBufferInfo& info);

	void setState(const State& state);
};