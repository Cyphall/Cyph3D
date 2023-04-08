#include "VKQueue.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Semaphore/VKSemaphore.h"
#include "Cyph3D/VKObject/VKSwapchain.h"
#include "Cyph3D/VKObject/Image/VKSwapchainImage.h"
#include "Cyph3D/VKObject/Fence/VKFence.h"

VKQueue::VKQueue(VKContext& context, uint32_t queueFamily, uint32_t queueIndex):
	VKObject(context), _queueFamily(queueFamily)
{
	_queue = _context.getDevice().getQueue(queueFamily, queueIndex);
}

const vk::Queue& VKQueue::getHandle()
{
	return _queue;
}

uint32_t VKQueue::getFamily() const
{
	return _queueFamily;
}

void VKQueue::submit(const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKSemaphore>* waitSemaphore, const VKPtr<VKSemaphore>* signalSemaphore)
{
	VKQueue::SubmitInfo& submitInfo = _submits.emplace_back();
	
	vk::SubmitInfo2 vkSubmitInfo;
	
	vk::SemaphoreSubmitInfo waitSemaphoreSubmitInfo;
	if (waitSemaphore != nullptr)
	{
		waitSemaphoreSubmitInfo.semaphore = (*waitSemaphore)->getHandle();
		waitSemaphoreSubmitInfo.value = 0;
		waitSemaphoreSubmitInfo.stageMask = vk::PipelineStageFlagBits2::eAllCommands;
		waitSemaphoreSubmitInfo.deviceIndex = 0;
		
		vkSubmitInfo.waitSemaphoreInfoCount = 1;
		vkSubmitInfo.pWaitSemaphoreInfos = &waitSemaphoreSubmitInfo;
		
		submitInfo.waitSemaphore = *waitSemaphore;
	}
	else
	{
		vkSubmitInfo.waitSemaphoreInfoCount = 0;
		vkSubmitInfo.pWaitSemaphoreInfos = nullptr;
	}
	
	vk::CommandBufferSubmitInfo commandBufferSubmitInfo;
	commandBufferSubmitInfo.commandBuffer = commandBuffer->getHandle();
	commandBufferSubmitInfo.deviceMask = 0;
	
	vkSubmitInfo.commandBufferInfoCount = 1;
	vkSubmitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;
	
	submitInfo.commandBuffer = commandBuffer;
	
	vk::SemaphoreSubmitInfo signalSemaphoreSubmitInfo;
	if (signalSemaphore != nullptr)
	{
		signalSemaphoreSubmitInfo.semaphore = (*signalSemaphore)->getHandle();
		signalSemaphoreSubmitInfo.value = 0;
		signalSemaphoreSubmitInfo.stageMask = vk::PipelineStageFlagBits2::eAllCommands;
		signalSemaphoreSubmitInfo.deviceIndex = 0;
		
		vkSubmitInfo.signalSemaphoreInfoCount = 1;
		vkSubmitInfo.pSignalSemaphoreInfos = &signalSemaphoreSubmitInfo;
		
		submitInfo.signalSemaphore = *signalSemaphore;
	}
	else
	{
		vkSubmitInfo.signalSemaphoreInfoCount = 0;
		vkSubmitInfo.pSignalSemaphoreInfos = nullptr;
	}
	
	commandBuffer->getStatusFence()->reset();
	_queue.submit2(vkSubmitInfo, commandBuffer->getStatusFence()->getHandle());
}

bool VKQueue::present(const VKPtr<VKSwapchainImage>& swapchainImage, const VKPtr<VKSemaphore>* waitSemaphore)
{
	uint32_t imageIndex = swapchainImage->getIndex();
	
	vk::PresentInfoKHR presentInfo;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchainImage->getSwapchain().getHandle();
	presentInfo.pImageIndices = &imageIndex;
	
	if (waitSemaphore != nullptr)
	{
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &waitSemaphore->get()->getHandle();
	}
	else
	{
		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = nullptr;
	}
	
	presentInfo.pResults = nullptr;
	
	uint64_t presentId = swapchainImage->getSwapchain().getNextPresentId();
	
	vk::PresentIdKHR vkPresentId;
	vkPresentId.swapchainCount = 1;
	vkPresentId.pPresentIds = &presentId;
	
	presentInfo.pNext = &vkPresentId;
	
	try
	{
		_queue.presentKHR(presentInfo);
	}
	catch (const vk::OutOfDateKHRError&)
	{
		return false;
	}
	
	swapchainImage->getSwapchain().onPresent();
	
	return true;
}

void VKQueue::handleCompletedSubmits()
{
	std::erase_if(_submits, [](VKQueue::SubmitInfo& submitInfo)
	{
		return submitInfo.commandBuffer->getStatusFence()->isSignaled();
	});
}