#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKFence : public VKObject
{
public:
	static std::shared_ptr<VKFence> create(VKContext& context, const vk::FenceCreateInfo& fenceCreateInfo);

	~VKFence() override;

	bool wait(uint64_t timeout = UINT64_MAX) const;
	bool isSignaled() const;

	void reset();

	const vk::Fence& getHandle();

private:
	VKFence(VKContext& context, const vk::FenceCreateInfo& fenceCreateInfo);

	vk::Fence _fence;
};