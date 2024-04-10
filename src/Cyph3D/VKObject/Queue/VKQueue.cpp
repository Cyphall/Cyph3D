#include "VKQueue.h"

#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Fence/VKFence.h"
#include "Cyph3D/VKObject/Image/VKSwapchainImage.h"
#include "Cyph3D/VKObject/Semaphore/VKSemaphore.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKSwapchain.h"

VKQueue::VKQueue(VKContext& context, uint32_t queueFamily, uint32_t queueIndex):
	VKObject(context),
	_queueFamily(queueFamily)
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

void VKQueue::submit(const VKPtr<VKCommandBuffer>& commandBuffer, vk::ArrayProxy<std::pair<VKPtr<VKSemaphore>, vk::PipelineStageFlags2>> waitSemaphores, vk::ArrayProxy<std::pair<VKPtr<VKSemaphore>, vk::PipelineStageFlags2>> signalSemaphores)
{
	std::scoped_lock lock(_mutex);

	std::vector<vk::SemaphoreSubmitInfo> waitSemaphoreSubmitInfos;
	for (const auto& [waitSemaphore, waitStage] : waitSemaphores)
	{
		vk::SemaphoreSubmitInfo& waitSemaphoreSubmitInfo = waitSemaphoreSubmitInfos.emplace_back();
		waitSemaphoreSubmitInfo.semaphore = waitSemaphore->getHandle();
		waitSemaphoreSubmitInfo.value = 0;
		waitSemaphoreSubmitInfo.stageMask = waitStage;
		waitSemaphoreSubmitInfo.deviceIndex = 0;
	}

	vk::CommandBufferSubmitInfo commandBufferSubmitInfo;
	commandBufferSubmitInfo.commandBuffer = commandBuffer->getHandle();
	commandBufferSubmitInfo.deviceMask = 0;

	std::vector<vk::SemaphoreSubmitInfo> signalSemaphoreSubmitInfos;
	for (const auto& [signalSemaphore, signalStage] : signalSemaphores)
	{
		vk::SemaphoreSubmitInfo& signalSemaphoreSubmitInfo = signalSemaphoreSubmitInfos.emplace_back();
		signalSemaphoreSubmitInfo.semaphore = signalSemaphore->getHandle();
		signalSemaphoreSubmitInfo.value = 0;
		signalSemaphoreSubmitInfo.stageMask = signalStage;
		signalSemaphoreSubmitInfo.deviceIndex = 0;
	}

	vk::SubmitInfo2 submitInfo;
	submitInfo.waitSemaphoreInfoCount = waitSemaphoreSubmitInfos.size();
	submitInfo.pWaitSemaphoreInfos = waitSemaphoreSubmitInfos.data();
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;
	submitInfo.signalSemaphoreInfoCount = signalSemaphoreSubmitInfos.size();
	submitInfo.pSignalSemaphoreInfos = signalSemaphoreSubmitInfos.data();

	commandBuffer->getStatusFence()->reset();
	_queue.submit2(submitInfo, commandBuffer->getStatusFence()->getHandle());

	auto extractSemaphore = [](const std::pair<VKPtr<VKSemaphore>, vk::PipelineStageFlags2>& pair)
	{
		return pair.first;
	};

	SubmitRecord& submitRecord = _submitRecords.emplace_back();
	submitRecord.commandBuffer = commandBuffer;
	submitRecord.waitSemaphores.resize(waitSemaphores.size());
	submitRecord.signalSemaphores.resize(signalSemaphores.size());
	std::ranges::transform(waitSemaphores, submitRecord.waitSemaphores.begin(), extractSemaphore);
	std::ranges::transform(signalSemaphores, submitRecord.signalSemaphores.begin(), extractSemaphore);
}

bool VKQueue::present(const VKPtr<VKSwapchainImage>& swapchainImage, vk::ArrayProxy<VKPtr<VKSemaphore>> waitSemaphores)
{
	std::scoped_lock lock(_mutex);

	std::vector<vk::Semaphore> waitVkSemaphores;
	for (const VKPtr<VKSemaphore>& semaphore : waitSemaphores)
	{
		waitVkSemaphores.push_back(semaphore->getHandle());
	}

	vk::PresentInfoKHR presentInfo;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchainImage->getSwapchain().getHandle();
	presentInfo.pImageIndices = &swapchainImage->getIndex();
	presentInfo.waitSemaphoreCount = waitVkSemaphores.size();
	presentInfo.pWaitSemaphores = waitVkSemaphores.data();
	presentInfo.pResults = nullptr;

	try
	{
		return _queue.presentKHR(presentInfo) == vk::Result::eSuccess;
	}
	catch (const vk::OutOfDateKHRError&)
	{
		return false;
	}
}

void VKQueue::handleCompletedSubmits()
{
	std::scoped_lock lock(_mutex);

	std::erase_if(
		_submitRecords,
		[](SubmitRecord& submitRecord)
		{
			return submitRecord.commandBuffer->getStatusFence()->isSignaled();
		}
	);
}