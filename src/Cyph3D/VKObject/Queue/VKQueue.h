#pragma once

#include "Cyph3D/VKObject/VKObject.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <mutex>
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

	void submit(const VKPtr<VKCommandBuffer>& commandBuffer, vk::ArrayProxy<VKPtr<VKSemaphore>> waitSemaphores, vk::ArrayProxy<VKPtr<VKSemaphore>> signalSemaphores);
	bool present(const VKPtr<VKSwapchainImage>& swapchainImage, vk::ArrayProxy<VKPtr<VKSemaphore>> waitSemaphores);

private:
	struct SubmitRecord
	{
		VKPtr<VKCommandBuffer> commandBuffer;
		std::vector<VKPtr<VKSemaphore>> waitSemaphores;
		std::vector<VKPtr<VKSemaphore>> signalSemaphores;
	};

	friend class VKContext;

	VKQueue(VKContext& context, uint32_t queueFamily, uint32_t queueIndex);

	void handleCompletedSubmits();

	uint32_t _queueFamily;
	vk::Queue _queue;
	std::mutex _mutex;

	std::vector<SubmitRecord> _submitRecords;
};