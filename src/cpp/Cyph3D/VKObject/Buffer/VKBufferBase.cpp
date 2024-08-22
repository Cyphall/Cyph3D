#include "VKBufferBase.h"

VKBufferBase::VKBufferBase(VKContext& context, const VKBufferInfo& info):
	VKObject(context),
	_info(info)
{
}

const VKBufferInfo& VKBufferBase::getInfo() const
{
	return _info;
}

vk::DeviceSize VKBufferBase::getByteSize() const
{
	return _info.getSize() * getStride();
}

const VKBufferBase::State& VKBufferBase::getState() const
{
	return _state;
}

void VKBufferBase::setState(const State& state)
{
	_state = state;
}