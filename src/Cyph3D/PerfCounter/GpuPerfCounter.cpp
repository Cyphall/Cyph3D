#include "GpuPerfCounter.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Query/VKTimestampQuery.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"

GpuPerfCounter::GpuPerfCounter()
{
	_query = VKDynamic<VKTimestampQuery>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKTimestampQuery::create(context);
	});
}

GpuPerfCounter::~GpuPerfCounter()
{

}

void GpuPerfCounter::start(const VKPtr<VKCommandBuffer>& commandBuffer)
{
	commandBuffer->beginTimestamp(_query.getCurrent());
}

void GpuPerfCounter::stop(const VKPtr<VKCommandBuffer>& commandBuffer)
{
	commandBuffer->endTimestamp(_query.getCurrent());
}

double GpuPerfCounter::retrieve()
{
	double elapsedTime = 0;
	if (_query->tryGetElapsedTime(elapsedTime))
	{
		_query.getCurrent()->resetTimestamps();
	}
	return elapsedTime;
}