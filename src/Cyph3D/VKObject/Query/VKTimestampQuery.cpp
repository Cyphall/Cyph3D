#include "VKTimestampQuery.h"

#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/VKContext.h"

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

double VKTimestampQuery::getElapsedTime() const
{
	if (!_isBeginInserted || !_isEndInserted)
		throw;

	// clang-format off
	std::array<uint64_t, 2> timestamps = _context.getDevice().getQueryPoolResult<std::array<uint64_t, 2>>(
		_queryPool,
		0,
		2,
		sizeof(uint64_t),
		vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait
	).value;
	// clang-format on

	double timestampDiff = timestamps[1] - timestamps[0];
	return static_cast<double>(timestampDiff) * static_cast<double>(_context.getProperties().limits.timestampPeriod) / 1000000.0;
}

const vk::QueryPool& VKTimestampQuery::getHandle()
{
	return _queryPool;
}

void VKTimestampQuery::resetTimestamps()
{
	_context.getDevice().resetQueryPool(_queryPool, 0, 2);
}

bool VKTimestampQuery::isInserted() const
{
	return _isBeginInserted && _isEndInserted;
}

void VKTimestampQuery::setIsBeginInserted(bool isInserted)
{
	_isBeginInserted = isInserted;
}

void VKTimestampQuery::setIsEndInserted(bool isInserted)
{
	_isEndInserted = isInserted;
}