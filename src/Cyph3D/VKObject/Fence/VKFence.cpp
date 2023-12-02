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
	switch (_context.getDevice().waitForFences(_fence, true, timeout))
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
	switch (_context.getDevice().getFenceStatus(_fence))
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