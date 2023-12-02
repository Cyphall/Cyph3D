#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKAccelerationStructureCompactedSizeQuery : public VKObject
{
public:
	static VKPtr<VKAccelerationStructureCompactedSizeQuery> create(VKContext& context);

	~VKAccelerationStructureCompactedSizeQuery() override;

	vk::DeviceSize getCompactedSize() const;
	bool tryGetCompactedSize(vk::DeviceSize& compactedSize) const;

	const vk::QueryPool& getHandle();

private:
	friend class VKCommandBuffer;

	explicit VKAccelerationStructureCompactedSizeQuery(VKContext& context);

	vk::QueryPool _queryPool;
};