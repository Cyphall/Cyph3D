#include "AssetManagerWorkerData.h"

#include <CyphGPU/CommandContext.hpp>

thread_local std::unique_ptr<cgpu::CommandContext> c3d::assetCommandContext;
