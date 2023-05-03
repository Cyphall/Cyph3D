#include "GpuPerfCounter.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Query/VKTimestampQuery.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"

GpuPerfCounter::GpuPerfCounter()
{
	_queryBegin = VKTimestampQuery::createDynamic(Engine::getVKContext());
	_queryEnd = VKTimestampQuery::createDynamic(Engine::getVKContext());
}

GpuPerfCounter::~GpuPerfCounter()
{

}

void GpuPerfCounter::start(const VKPtr<VKCommandBuffer>& commandBuffer)
{
	commandBuffer->insertTimestamp(_queryBegin.getVKPtr());
}

void GpuPerfCounter::stop(const VKPtr<VKCommandBuffer>& commandBuffer)
{
	commandBuffer->insertTimestamp(_queryEnd.getVKPtr());
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
		commandBuffer->resetTimestamp(_queryBegin.getVKPtr());
	}
	if (_queryEnd->isInserted())
	{
		commandBuffer->resetTimestamp(_queryEnd.getVKPtr());
	}
	
	return timeDiff / 1000000.0;
}