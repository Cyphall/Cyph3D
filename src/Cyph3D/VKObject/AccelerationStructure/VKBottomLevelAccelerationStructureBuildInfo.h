#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <vector>

class VKBufferBase;

struct VKBottomLevelAccelerationStructureBuildInfo
{
	VKPtr<VKBufferBase> vertexBuffer;
	vk::Format vertexFormat;
	size_t vertexStride;
	VKPtr<VKBufferBase> indexBuffer;
	vk::IndexType indexType;
};