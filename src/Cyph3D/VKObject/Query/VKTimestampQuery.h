#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKTimestampQuery : public VKObject
{
public:
	static VKPtr<VKTimestampQuery> create(VKContext& context);
	
	~VKTimestampQuery() override;
	
	uint64_t getTimestamp() const;
	bool tryGetTimestamp(uint64_t& timestamp) const;
	
	const vk::QueryPool& getHandle();
	
	bool isInserted() const;

private:
	friend class VKCommandBuffer;
	
	explicit VKTimestampQuery(VKContext& context);
	
	void setIsInserted(bool isInserted);
	
	vk::QueryPool _queryPool;
	
	bool _isInserted = false;
};