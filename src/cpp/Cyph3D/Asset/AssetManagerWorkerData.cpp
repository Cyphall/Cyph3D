#include "AssetManagerWorkerData.h"

thread_local std::shared_ptr<VKCommandBuffer> assetGraphicsCommandBuffer;
thread_local std::shared_ptr<VKCommandBuffer> assetComputeCommandBuffer;
thread_local std::shared_ptr<VKCommandBuffer> assetTransferCommandBuffer;