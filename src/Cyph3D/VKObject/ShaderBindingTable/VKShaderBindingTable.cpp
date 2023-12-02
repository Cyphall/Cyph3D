#include "VKShaderBindingTable.h"

#include "Cyph3D/VKObject/VKHelper.h"

VKPtr<VKShaderBindingTable> VKShaderBindingTable::create(VKContext& context, const VKShaderBindingTableInfo& info)
{
	return VKPtr<VKShaderBindingTable>(new VKShaderBindingTable(context, info));
}

VKShaderBindingTable::VKShaderBindingTable(VKContext& context, const VKShaderBindingTableInfo& info):
	VKObject(context),
	_info(info)
{
	// calculate buffer size

	// raygen sbt

	uint64_t raygenSBTOffset = VKHelper::alignUp(0, context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	_raygenSBTStride = VKHelper::alignUp(_info.getRaygenRecord().size(), context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	_raygenSBTSize = _raygenSBTStride;

	// triangle hit sbt

	uint64_t triangleHitSBTOffset = VKHelper::alignUp(raygenSBTOffset + _raygenSBTSize, context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	_triangleHitSBTStride = VKHelper::alignUp(_info.getMaxTriangleHitRecordSize(), context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	for (const std::vector<std::vector<std::byte>>& recordGroup : _info.getTriangleHitRecords())
	{
		_triangleHitSBTSize += _triangleHitSBTStride * recordGroup.size();
	}

	// miss sbt

	uint64_t missSBTOffset = VKHelper::alignUp(triangleHitSBTOffset + _triangleHitSBTSize, context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	_missSBTStride = VKHelper::alignUp(_info.getMaxMissRecordSize(), context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	_missSBTSize += _missSBTStride * _info.getMissRecords().size();

	// create buffer

	VKBufferInfo bufferInfo(
		missSBTOffset + _missSBTSize,
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR
	);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	bufferInfo.setRequiredAlignment(_context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	bufferInfo.setName("SBT buffer");

	_buffer = VKBuffer<std::byte>::create(context, bufferInfo);

	_raygenSBTAddress = _buffer->getDeviceAddress() + raygenSBTOffset;
	_triangleHitSBTAddress = _buffer->getDeviceAddress() + triangleHitSBTOffset;
	_missSBTAddress = _buffer->getDeviceAddress() + missSBTOffset;

	// copy data to buffer

	// raygen sbt

	std::byte* raygenBufferPtr = _buffer->getHostPointer() + raygenSBTOffset;
	std::memcpy(raygenBufferPtr, _info.getRaygenRecord().data(), _info.getRaygenRecord().size());

	// triangle hit sbt

	std::byte* triangleHitBufferPtr = _buffer->getHostPointer() + triangleHitSBTOffset;
	for (const std::vector<std::vector<std::byte>>& recordGroup : _info.getTriangleHitRecords())
	{
		for (const std::vector<std::byte>& record : recordGroup)
		{
			std::memcpy(triangleHitBufferPtr, record.data(), record.size());
			triangleHitBufferPtr += _triangleHitSBTStride;
		}
	}

	// miss sbt

	std::byte* missBufferPtr = _buffer->getHostPointer() + missSBTOffset;
	for (const std::vector<std::byte>& record : _info.getMissRecords())
	{
		std::memcpy(missBufferPtr, record.data(), record.size());
		missBufferPtr += _missSBTStride;
	}
}

VKShaderBindingTable::~VKShaderBindingTable() = default;

const VKShaderBindingTableInfo& VKShaderBindingTable::getInfo() const
{
	return _info;
}

const vk::DeviceAddress& VKShaderBindingTable::getRaygenSBTAddress() const
{
	return _raygenSBTAddress;
}

const vk::DeviceSize& VKShaderBindingTable::getRaygenSBTSize() const
{
	return _raygenSBTSize;
}

const vk::DeviceSize& VKShaderBindingTable::getRaygenSBTStride() const
{
	return _raygenSBTStride;
}

const vk::DeviceAddress& VKShaderBindingTable::getTriangleHitSBTAddress() const
{
	return _triangleHitSBTAddress;
}

const vk::DeviceSize& VKShaderBindingTable::getTriangleHitSBTSize() const
{
	return _triangleHitSBTSize;
}

const vk::DeviceSize& VKShaderBindingTable::getTriangleHitSBTStride() const
{
	return _triangleHitSBTStride;
}

const vk::DeviceAddress& VKShaderBindingTable::getMissSBTAddress() const
{
	return _missSBTAddress;
}

const vk::DeviceSize& VKShaderBindingTable::getMissSBTSize() const
{
	return _missSBTSize;
}

const vk::DeviceSize& VKShaderBindingTable::getMissSBTStride() const
{
	return _missSBTStride;
}