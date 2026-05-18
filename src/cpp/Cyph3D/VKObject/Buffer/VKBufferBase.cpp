#include "VKBufferBase.h"

c3d::VKBufferBase::VKBufferBase(VKContext& context, const VKBufferInfo& info):
	VKObject(context),
	_info(info)
{
}

const c3d::VKBufferInfo& c3d::VKBufferBase::getInfo() const
{
	return _info;
}

vk::DeviceSize c3d::VKBufferBase::getByteSize() const
{
	return _info.getSize() * getStride();
}

const c3d::VKBufferBase::State& c3d::VKBufferBase::getState() const
{
	return _state;
}

void c3d::VKBufferBase::setState(const State& state)
{
	_state = state;
}