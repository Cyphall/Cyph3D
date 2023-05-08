#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <cstddef>
#include <vulkan/vulkan.hpp>

template<typename T>
class VKResizableBuffer;
class VKContext;
class VKRayTracingPipeline;

class VKHelper
{
public:
	static size_t alignUp(size_t size, size_t alignment);
	
	static void buildRaygenShaderBindingTable(VKContext& context, const VKPtr<VKRayTracingPipeline>& pipeline, const VKPtr<VKResizableBuffer<std::byte>>& destBuffer);
	static void buildMissShaderBindingTable(VKContext& context, const VKPtr<VKRayTracingPipeline>& pipeline, const VKPtr<VKResizableBuffer<std::byte>>& destBuffer);
	static void buildHitShaderBindingTable(VKContext& context, const VKPtr<VKRayTracingPipeline>& pipeline, const VKPtr<VKResizableBuffer<std::byte>>& destBuffer);
};