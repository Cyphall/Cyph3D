#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

class VKCommandBuffer;

struct AssetManagerWorkerData
{
	VKPtr<VKCommandBuffer> graphicsCommandBuffer;
	VKPtr<VKCommandBuffer> computeCommandBuffer;
	VKPtr<VKCommandBuffer> transferCommandBuffer;
};