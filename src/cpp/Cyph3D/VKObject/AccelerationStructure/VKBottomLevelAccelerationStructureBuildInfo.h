#pragma once

#include <vulkan/vulkan.hpp>

namespace c3d
{
class VKBufferBase;

struct VKBottomLevelAccelerationStructureBuildInfo
{
	std::shared_ptr<VKBufferBase> vertexBuffer;
	vk::Format vertexFormat;
	size_t vertexStride;
	std::shared_ptr<VKBufferBase> indexBuffer;
	vk::IndexType indexType;
};
}