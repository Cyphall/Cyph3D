#pragma once

#include "Cyph3D/VKObject/VKObject.h"

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

	void submit(const std::shared_ptr<VKCommandBuffer>& commandBuffer, vk::ArrayProxy<std::pair<std::shared_ptr<VKSemaphore>, vk::PipelineStageFlags2>> waitSemaphores, vk::ArrayProxy<std::pair<std::shared_ptr<VKSemaphore>, vk::PipelineStageFlags2>> signalSemaphores);
	bool present(const std::shared_ptr<VKSwapchainImage>& swapchainImage, vk::ArrayProxy<std::shared_ptr<VKSemaphore>> waitSemaphores);

private:
	struct SubmitRecord
	{
		std::shared_ptr<VKCommandBuffer> commandBuffer;
		std::vector<std::shared_ptr<VKSemaphore>> waitSemaphores;
		std::vector<std::shared_ptr<VKSemaphore>> signalSemaphores;
	};

	friend class VKContext;

	VKQueue(VKContext& context, uint32_t queueFamily, uint32_t queueIndex);

	void handleCompletedSubmits();

	uint32_t _queueFamily;
	vk::Queue _queue;
	std::mutex _mutex;

	std::vector<SubmitRecord> _submitRecords;
};