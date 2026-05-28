#include "VKFence.h"

#include <Cyph3D/VKObject/VKContext.h>

std::shared_ptr<c3d::VKFence> c3d::VKFence::create(VKContext& context, const vk::FenceCreateInfo& fenceCreateInfo)
{
	return std::shared_ptr<VKFence>(new VKFence(context, fenceCreateInfo));
}

c3d::VKFence::VKFence(VKContext& context, const vk::FenceCreateInfo& fenceCreateInfo):
	VKObject(context)
{
	_fence = _context.getDevice().createFence(fenceCreateInfo);
}

c3d::VKFence::~VKFence()
{
	_context.getDevice().destroyFence(_fence);
}

bool c3d::VKFence::wait(uint64_t timeout) const
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

bool c3d::VKFence::isSignaled() const
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

void c3d::VKFence::reset()
{
	_context.getDevice().resetFences(_fence);
}

const vk::Fence& c3d::VKFence::getHandle()
{
	return _fence;
}