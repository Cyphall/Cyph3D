#pragma once

#include <CyphGPU/fwd.hpp>
#include <memory>

namespace c3d
{
extern thread_local std::unique_ptr<cgpu::CommandContext> assetCommandContext;
}
