#include "VKAccelerationStructureCompactedSizeQuery.h"

#include <Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h>
#include <Cyph3D/VKObject/VKContext.h>

std::shared_ptr<c3d::VKAccelerationStructureCompactedSizeQuery> c3d::VKAccelerationStructureCompactedSizeQuery::create(VKContext& context)
{
	return std::shared_ptr<VKAccelerationStructureCompactedSizeQuery>(new VKAccelerationStructureCompactedSizeQuery(context));
}

c3d::VKAccelerationStructureCompactedSizeQuery::VKAccelerationStructureCompactedSizeQuery(VKContext& context):
	VKObject(context)
{
	vk::QueryPoolCreateInfo queryPoolCreateInfo;
	queryPoolCreateInfo.queryType = vk::QueryType::eAccelerationStructureCompactedSizeKHR;
	queryPoolCreateInfo.queryCount = 1;

	_queryPool = _context.getDevice().createQueryPool(queryPoolCreateInfo);

	_context.getDevice().resetQueryPool(_queryPool, 0, 1);
}

c3d::VKAccelerationStructureCompactedSizeQuery::~VKAccelerationStructureCompactedSizeQuery()
{
	_context.getDevice().destroyQueryPool(_queryPool);
}

vk::DeviceSize c3d::VKAccelerationStructureCompactedSizeQuery::getCompactedSize() const
{
	auto [result, data] = _context.getDevice().getQueryPoolResult<vk::DeviceSize>(
		_queryPool,
		0,
		1,
		sizeof(vk::DeviceSize),
		vk::QueryResultFlagBits::e64 | vk::QueryResultFlagBits::eWait
	);

	return data;
}

bool c3d::VKAccelerationStructureCompactedSizeQuery::tryGetCompactedSize(vk::DeviceSize& compactedSize) const
{
	auto [result, data] = _context.getDevice().getQueryPoolResult<vk::DeviceSize>(
		_queryPool,
		0,
		1,
		sizeof(vk::DeviceSize),
		vk::QueryResultFlagBits::e64
	);

	if (result == vk::Result::eSuccess)
	{
		compactedSize = data;
		return true;
	}
	else if (result == vk::Result::eNotReady)
	{
		return false;
	}

	throw;
}

const vk::QueryPool& c3d::VKAccelerationStructureCompactedSizeQuery::getHandle()
{
	return _queryPool;
}