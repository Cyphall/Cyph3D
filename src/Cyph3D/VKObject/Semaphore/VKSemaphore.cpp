#include "VKSemaphore.h"

#include "Cyph3D/VKObject/VKContext.h"

std::shared_ptr<VKSemaphore> VKSemaphore::create(VKContext& context, const vk::SemaphoreCreateInfo& semaphoreCreateInfo)
{
	return std::shared_ptr<VKSemaphore>(new VKSemaphore(context, semaphoreCreateInfo));
}

VKSemaphore::VKSemaphore(VKContext& context, const vk::SemaphoreCreateInfo& semaphoreCreateInfo):
	VKObject(context)
{
	_semaphore = _context.getDevice().createSemaphore(semaphoreCreateInfo);
}

VKSemaphore::~VKSemaphore()
{
	_context.getDevice().destroySemaphore(_semaphore);
}

const vk::Semaphore& VKSemaphore::getHandle()
{
	return _semaphore;
}