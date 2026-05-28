#include "VKShaderBindingTable.h"

#include <Cyph3D/VKObject/VKHelper.h>

std::shared_ptr<c3d::VKShaderBindingTable> c3d::VKShaderBindingTable::create(VKContext& context, const VKShaderBindingTableInfo& info)
{
	return std::shared_ptr<VKShaderBindingTable>(new VKShaderBindingTable(context, info));
}

c3d::VKShaderBindingTable::VKShaderBindingTable(VKContext& context, const VKShaderBindingTableInfo& info):
	VKObject(context),
	_info(info)
{
	// calculate SBT sizes

	uint32_t groupHandleAlignment = context.getRayTracingPipelineProperties().shaderGroupHandleAlignment;

	_raygenSBTStride = VKHelper::alignUp(_info.getMaxRaygenRecordSize(), groupHandleAlignment);
	_raygenSBTSize = _raygenSBTStride;

	_triangleHitSBTStride = VKHelper::alignUp(_info.getMaxTriangleHitRecordSize(), groupHandleAlignment);
	_triangleHitSBTSize = _triangleHitSBTStride * _info.getTriangleHitRecords().size();

	_missSBTStride = VKHelper::alignUp(_info.getMaxMissRecordSize(), groupHandleAlignment);
	_missSBTSize = _missSBTStride * _info.getTriangleHitRecords().size();

	// calculate SBT offsets

	uint32_t groupBaseAlignment = context.getRayTracingPipelineProperties().shaderGroupBaseAlignment;

	vk::DeviceSize rangeOffset = 0;
	auto allocRange = [&](vk::DeviceSize size, vk::DeviceSize alignment) -> vk::DeviceSize
	{
		vk::DeviceSize alignedOffset = VKHelper::alignUp(rangeOffset, alignment);

		rangeOffset = alignedOffset + size;

		return alignedOffset;
	};

	vk::DeviceSize raygenSBTOffset = allocRange(_raygenSBTSize, groupBaseAlignment);
	vk::DeviceSize triangleHitSBTOffset = allocRange(_triangleHitSBTSize, groupBaseAlignment);
	vk::DeviceSize missSBTOffset = allocRange(_missSBTSize, groupBaseAlignment);

	// create buffer

	VKBufferInfo bufferInfo(
		rangeOffset,
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR
	);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	bufferInfo.setRequiredAlignment(groupBaseAlignment);
	bufferInfo.setName("SBT buffer");

	_buffer = VKBuffer<std::byte>::create(context, bufferInfo);

	_raygenSBTAddress = _buffer->getDeviceAddress() + raygenSBTOffset;
	_triangleHitSBTAddress = _buffer->getDeviceAddress() + triangleHitSBTOffset;
	_missSBTAddress = _buffer->getDeviceAddress() + missSBTOffset;

	// copy data to buffer

	auto writeRecord = [](const VKShaderBindingTableInfo::Record& record, vk::DeviceSize stride, std::byte*& dst)
	{
		std::memcpy(dst, record.groupHandle.data(), record.groupHandle.size());
		std::memcpy(dst + record.groupHandle.size(), record.uniforms.data(), record.uniforms.size());
		dst += stride;
	};

	std::byte* raygenSBTPtr = _buffer->getHostPointer() + raygenSBTOffset;
	writeRecord(_info.getRaygenRecord(), _raygenSBTStride, raygenSBTPtr);

	std::byte* triangleHitSBTPtr = _buffer->getHostPointer() + triangleHitSBTOffset;
	for (const VKShaderBindingTableInfo::Record& record : _info.getTriangleHitRecords())
		writeRecord(record, _triangleHitSBTStride, triangleHitSBTPtr);

	std::byte* missSBTPtr = _buffer->getHostPointer() + missSBTOffset;
	for (const VKShaderBindingTableInfo::Record& record : _info.getMissRecords())
		writeRecord(record, _missSBTStride, missSBTPtr);
}

c3d::VKShaderBindingTable::~VKShaderBindingTable() = default;

const c3d::VKShaderBindingTableInfo& c3d::VKShaderBindingTable::getInfo() const
{
	return _info;
}

const std::shared_ptr<c3d::VKBuffer<std::byte>>& c3d::VKShaderBindingTable::getBuffer() const
{
	return _buffer;
}

const vk::DeviceAddress& c3d::VKShaderBindingTable::getRaygenSBTAddress() const
{
	return _raygenSBTAddress;
}

const vk::DeviceSize& c3d::VKShaderBindingTable::getRaygenSBTSize() const
{
	return _raygenSBTSize;
}

const vk::DeviceSize& c3d::VKShaderBindingTable::getRaygenSBTStride() const
{
	return _raygenSBTStride;
}

const vk::DeviceAddress& c3d::VKShaderBindingTable::getTriangleHitSBTAddress() const
{
	return _triangleHitSBTAddress;
}

const vk::DeviceSize& c3d::VKShaderBindingTable::getTriangleHitSBTSize() const
{
	return _triangleHitSBTSize;
}

const vk::DeviceSize& c3d::VKShaderBindingTable::getTriangleHitSBTStride() const
{
	return _triangleHitSBTStride;
}

const vk::DeviceAddress& c3d::VKShaderBindingTable::getMissSBTAddress() const
{
	return _missSBTAddress;
}

const vk::DeviceSize& c3d::VKShaderBindingTable::getMissSBTSize() const
{
	return _missSBTSize;
}

const vk::DeviceSize& c3d::VKShaderBindingTable::getMissSBTStride() const
{
	return _missSBTStride;
}