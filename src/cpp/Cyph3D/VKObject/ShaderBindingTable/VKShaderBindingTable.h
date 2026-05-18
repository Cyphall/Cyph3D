#pragma once

#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTableInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <memory>
#include <vulkan/vulkan.hpp>

namespace c3d
{
class VKShaderBindingTable : public VKObject
{
public:
	static std::shared_ptr<VKShaderBindingTable> create(VKContext& context, const VKShaderBindingTableInfo& info);

	~VKShaderBindingTable() override;

	const VKShaderBindingTableInfo& getInfo() const;

	const std::shared_ptr<VKBuffer<std::byte>>& getBuffer() const;

	const vk::DeviceAddress& getRaygenSBTAddress() const;
	const vk::DeviceSize& getRaygenSBTSize() const;
	const vk::DeviceSize& getRaygenSBTStride() const;
	const vk::DeviceAddress& getTriangleHitSBTAddress() const;
	const vk::DeviceSize& getTriangleHitSBTSize() const;
	const vk::DeviceSize& getTriangleHitSBTStride() const;
	const vk::DeviceAddress& getMissSBTAddress() const;
	const vk::DeviceSize& getMissSBTSize() const;
	const vk::DeviceSize& getMissSBTStride() const;

private:
	VKShaderBindingTable(VKContext& context, const VKShaderBindingTableInfo& info);

	VKShaderBindingTableInfo _info;

	std::shared_ptr<VKBuffer<std::byte>> _buffer;

	vk::DeviceAddress _raygenSBTAddress;
	vk::DeviceSize _raygenSBTSize;
	vk::DeviceSize _raygenSBTStride;

	vk::DeviceAddress _triangleHitSBTAddress;
	vk::DeviceSize _triangleHitSBTSize;
	vk::DeviceSize _triangleHitSBTStride;

	vk::DeviceAddress _missSBTAddress;
	vk::DeviceSize _missSBTSize;
	vk::DeviceSize _missSBTStride;
};
}