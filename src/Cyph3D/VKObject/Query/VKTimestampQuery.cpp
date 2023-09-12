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
	
	resetTimestamps();
}

VKTimestampQuery::~VKTimestampQuery()
{
	_context.getDevice().destroyQueryPool(_queryPool);
}

bool VKTimestampQuery::tryGetElapsedTime(double& elapsedTime) const
{
	if (!_isBeginInserted || !_isEndInserted)
		return false;
	
	auto [result, timestamps] = _context.getDevice().getQueryPoolResult<std::array<uint64_t, 2>>(
		_queryPool,
		0,
		2,
		sizeof(uint64_t),
		vk::QueryResultFlagBits::e64);
	
	if (result == vk::Result::eSuccess)
	{
		double timestampDiff = timestamps[1] - timestamps[0];
		elapsedTime = static_cast<double>(timestampDiff) * static_cast<double>(_context.getProperties().limits.timestampPeriod) / 1000000.0;
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

void VKTimestampQuery::resetTimestamps()
{
	_context.getDevice().resetQueryPool(_queryPool, 0, 2);
}

void VKTimestampQuery::setIsBeginInserted(bool isInserted)
{
	_isBeginInserted = isInserted;
}

void VKTimestampQuery::setIsEndInserted(bool isInserted)
{
	_isEndInserted = isInserted;
}