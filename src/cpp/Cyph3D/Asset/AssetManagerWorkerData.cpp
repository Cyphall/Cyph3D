#include "AssetManagerWorkerData.h"

thread_local std::shared_ptr<c3d::VKCommandBuffer> c3d::assetGraphicsCommandBuffer;
thread_local std::shared_ptr<c3d::VKCommandBuffer> c3d::assetComputeCommandBuffer;
thread_local std::shared_ptr<c3d::VKCommandBuffer> c3d::assetTransferCommandBuffer;