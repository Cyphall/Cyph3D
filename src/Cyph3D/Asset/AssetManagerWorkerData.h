#pragma once


class VKCommandBuffer;

struct AssetManagerWorkerData
{
	std::shared_ptr<VKCommandBuffer> graphicsCommandBuffer;
	std::shared_ptr<VKCommandBuffer> computeCommandBuffer;
	std::shared_ptr<VKCommandBuffer> transferCommandBuffer;
};