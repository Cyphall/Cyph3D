#include "VKSemaphore.h"

#include "Cyph3D/VKObject/VKContext.h"

std::shared_ptr<c3d::VKSemaphore> c3d::VKSemaphore::create(VKContext& context, const vk::SemaphoreCreateInfo& semaphoreCreateInfo)
{
	return std::shared_ptr<VKSemaphore>(new VKSemaphore(context, semaphoreCreateInfo));
}

c3d::VKSemaphore::VKSemaphore(VKContext& context, const vk::SemaphoreCreateInfo& semaphoreCreateInfo):
	VKObject(context)
{
	_semaphore = _context.getDevice().createSemaphore(semaphoreCreateInfo);
}

c3d::VKSemaphore::~VKSemaphore()
{
	_context.getDevice().destroySemaphore(_semaphore);
}

const vk::Semaphore& c3d::VKSemaphore::getHandle()
{
	return _semaphore;
}