#include "VKSemaphore.h"

#include "Cyph3D/VKObject/VKContext.h"

VKPtr<VKSemaphore> VKSemaphore::create(VKContext& context, const vk::SemaphoreCreateInfo& semaphoreCreateInfo)
{
	return VKPtr<VKSemaphore>(new VKSemaphore(context, semaphoreCreateInfo));
}

VKDynamic<VKSemaphore> VKSemaphore::createDynamic(VKContext& context, const vk::SemaphoreCreateInfo& semaphoreCreateInfo)
{
	return VKDynamic<VKSemaphore>(context, semaphoreCreateInfo);
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