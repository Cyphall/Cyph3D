#pragma once

#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTableInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKShaderBindingTable : public VKObject
{
public:
	static VKPtr<VKShaderBindingTable> create(VKContext& context, const VKShaderBindingTableInfo& info);
	
	~VKShaderBindingTable() override;
	
	const VKShaderBindingTableInfo& getInfo() const;
	
	const VKPtr<VKBuffer<std::byte>>& getBuffer() const;
	
	const vk::DeviceAddress& getRaygenSBTAddress() const;
	const vk::DeviceSize& getRaygenSBTAlignedSize() const;
	const vk::DeviceSize& getRaygenSBTStride() const;
	const vk::DeviceAddress& getTriangleHitSBTAddress() const;
	const vk::DeviceSize& getTriangleHitSBTAlignedSize() const;
	const vk::DeviceSize& getTriangleHitSBTStride() const;
	const vk::DeviceAddress& getMissSBTAddress() const;
	const vk::DeviceSize& getMissSBTAlignedSize() const;
	const vk::DeviceSize& getMissSBTStride() const;

private:
	VKShaderBindingTable(VKContext& context, const VKShaderBindingTableInfo& info);
	
	VKShaderBindingTableInfo _info;
	
	VKPtr<VKBuffer<std::byte>> _buffer;
	
	vk::DeviceAddress _raygenSBTAddress = 0;
	vk::DeviceSize _raygenSBTAlignedSize = 0;
	vk::DeviceSize _raygenSBTStride = 0;
	vk::DeviceAddress _triangleHitSBTAddress = 0;
	vk::DeviceSize _triangleHitSBTAlignedSize = 0;
	vk::DeviceSize _triangleHitSBTStride = 0;
	vk::DeviceAddress _missSBTAddress = 0;
	vk::DeviceSize _missSBTAlignedSize = 0;
	vk::DeviceSize _missSBTStride = 0;
};