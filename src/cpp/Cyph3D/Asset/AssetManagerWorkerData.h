#pragma once

#include <memory>

namespace c3d
{
class VKCommandBuffer;

extern thread_local std::shared_ptr<VKCommandBuffer> assetGraphicsCommandBuffer;
extern thread_local std::shared_ptr<VKCommandBuffer> assetComputeCommandBuffer;
extern thread_local std::shared_ptr<VKCommandBuffer> assetTransferCommandBuffer;
}