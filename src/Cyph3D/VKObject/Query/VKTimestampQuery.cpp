#include "VKTimestampQuery.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"

VKPtr<VKTimestampQuery> VKTimestampQuery::create(VKContext& context)
{
	return VKPtr<VKTimestampQuery>(new VKTimestampQuery(context));
}

VKTimestampQuery::VKTimestampQuery(VKContext& context):
	VKObject(context)
{
	vk::QueryPoolCreateInfo queryPoolCreateInfo;
	queryPoolCreateInfo.queryType = vk::QueryType::eTimestamp;
	queryPoolCreateInfo.queryCount = 2;
	
	_queryPool = _context.getDevice().createQueryPool(queryPoolCreateInfo);
	
	_context.getDevice().resetQueryPool(_queryPool, 0, 1);
}

VKTimestampQuery::~VKTimestampQuery()
{
	_context.getDevice().destroyQueryPool(_queryPool);
}

uint64_t VKTimestampQuery::getTimestamp() const
{
	auto [result, timestamp] = _context.getDevice().getQueryPoolResult<uint64_t>(
		_queryPool,
		0,
		1,
		sizeof(uint64_t),
		vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait);
	
	return timestamp;
}

bool VKTimestampQuery::tryGetTimestamp(uint64_t& timestamp) const
{
	auto [result, data] = _context.getDevice().getQueryPoolResult<uint64_t>(
		_queryPool,
		0,
		1,
		sizeof(uint64_t),
		vk::QueryResultFlagBits::e64);
	
	if (result == vk::Result::eSuccess)
	{
		timestamp = data;
		return true;
	}
	else if (result == vk::Result::eNotReady)
	{
		return false;
	}
	
	throw;
}

const vk::QueryPool& VKTimestampQuery::getHandle()
{
	return _queryPool;
}

bool VKTimestampQuery::isInserted() const
{
	return _isInserted;
}

void VKTimestampQuery::setIsInserted(bool isInserted)
{
	_isInserted = isInserted;
}