#include "GpuPerfCounter.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Query/VKTimestampQuery.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"

GpuPerfCounter::GpuPerfCounter()
{
	_queryBegin = VKDynamic<VKTimestampQuery>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKTimestampQuery::create(context);
	});
	
	_queryEnd = VKDynamic<VKTimestampQuery>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKTimestampQuery::create(context);
	});
}

GpuPerfCounter::~GpuPerfCounter()
{

}

void GpuPerfCounter::start(const VKPtr<VKCommandBuffer>& commandBuffer)
{
	commandBuffer->insertTimestamp(_queryBegin.getCurrent());
}

void GpuPerfCounter::stop(const VKPtr<VKCommandBuffer>& commandBuffer)
{
	commandBuffer->insertTimestamp(_queryEnd.getCurrent());
}

double GpuPerfCounter::retrieve(const VKPtr<VKCommandBuffer>& commandBuffer)
{
	double timeDiff = 0;
	if (_queryBegin->isInserted() && _queryEnd->isInserted())
	{
		uint64_t timestampBegin;
		if (!_queryBegin->tryGetTimestamp(timestampBegin))
		{
			throw;
		}
		
		uint64_t timestampEnd;
		if (!_queryEnd->tryGetTimestamp(timestampEnd))
		{
			throw;
		}
		
		double timestampDiff = timestampEnd - timestampBegin;
		timeDiff = timestampDiff * Engine::getVKContext().getProperties().limits.timestampPeriod;
	}
	
	if (_queryBegin->isInserted())
	{
		commandBuffer->resetTimestamp(_queryBegin.getCurrent());
	}
	if (_queryEnd->isInserted())
	{
		commandBuffer->resetTimestamp(_queryEnd.getCurrent());
	}
	
	return timeDiff / 1000000.0;
}