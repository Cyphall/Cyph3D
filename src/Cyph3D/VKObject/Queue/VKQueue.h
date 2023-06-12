#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKFence;
class VKCommandBuffer;
class VKSemaphore;
class VKSwapchainImage;

class VKQueue : public VKObject
{
public:
	const vk::Queue& getHandle();
	
	uint32_t getFamily() const;
	
	void submit(const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKSemaphore>* waitSemaphore, const VKPtr<VKSemaphore>* signalSemaphore);
	bool present(const VKPtr<VKSwapchainImage>& swapchainImage, const VKPtr<VKSemaphore>* waitSemaphore);

private:
	struct SubmitInfo
	{
		VKPtr<VKCommandBuffer> commandBuffer;
		VKPtr<VKSemaphore> waitSemaphore;
		VKPtr<VKSemaphore> signalSemaphore;
	};
	
	friend class VKContext;
	
	VKQueue(VKContext& context, uint32_t queueFamily, uint32_t queueIndex);
	
	void handleCompletedSubmits();
	
	uint32_t _queueFamily;
	vk::Queue _queue;
	std::mutex _mutex;
	
	std::vector<SubmitInfo> _submits;
};