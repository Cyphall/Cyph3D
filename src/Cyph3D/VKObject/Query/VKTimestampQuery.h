#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKTimestampQuery : public VKObject
{
public:
	static VKPtr<VKTimestampQuery> create(VKContext& context);
	
	~VKTimestampQuery() override;
	
	bool tryGetElapsedTime(double& elapsedTime) const;
	
	const vk::QueryPool& getHandle();
	
	void resetTimestamps();

private:
	friend class VKCommandBuffer;
	
	explicit VKTimestampQuery(VKContext& context);
	
	void setIsBeginInserted(bool isInserted);
	void setIsEndInserted(bool isInserted);
	
	vk::QueryPool _queryPool;
	
	bool _isBeginInserted = false;
	bool _isEndInserted = false;
};