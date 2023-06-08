#include "VKFence.h"

#include "Cyph3D/VKObject/VKContext.h"

VKPtr<VKFence> VKFence::create(VKContext& context, const vk::FenceCreateInfo& fenceCreateInfo)
{
	return VKPtr<VKFence>(new VKFence(context, fenceCreateInfo));
}

VKFence::VKFence(VKContext& context, const vk::FenceCreateInfo& fenceCreateInfo):
	VKObject(context)
{
	_fence = _context.getDevice().createFence(fenceCreateInfo);
}

VKFence::~VKFence()
{
	_context.getDevice().destroyFence(_fence);
}

bool VKFence::wait(uint64_t timeout) const
{
	vk::Result result = _context.getDevice().waitForFences(_fence, true, timeout);
	switch (result)
	{
		case vk::Result::eSuccess:
			return true;
		case vk::Result::eTimeout:
			return false;
		default:
			throw;
	}
}

bool VKFence::isSignaled() const
{
	vk::Result status = _context.getDevice().getFenceStatus(_fence);
	switch (status)
	{
		case vk::Result::eSuccess:
			return true;
		case vk::Result::eNotReady:
			return false;
		default:
			throw;
	}
}

void VKFence::reset()
{
	_context.getDevice().resetFences(_fence);
}

const vk::Fence& VKFence::getHandle()
{
	return _fence;
}