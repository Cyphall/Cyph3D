#pragma once

#include "Cyph3D/VKObject/VKDynamic.h"

class VKTimestampQuery;

class GpuPerfCounter
{
public:
	GpuPerfCounter();
	~GpuPerfCounter();

	void start(const VKPtr<VKCommandBuffer>& commandBuffer);
	void stop(const VKPtr<VKCommandBuffer>& commandBuffer);

	double retrieve();

private:
	VKDynamic<VKTimestampQuery> _query;
};
