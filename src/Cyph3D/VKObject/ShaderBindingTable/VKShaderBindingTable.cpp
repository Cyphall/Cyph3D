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
	_raygenSBTStride = VKHelper::alignUp(_info.getRaygenRecord().size(), context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	_triangleHitSBTStride = VKHelper::alignUp(_info.getMaxTriangleHitRecordSize(), context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	_missSBTStride = VKHelper::alignUp(_info.getMaxMissRecordSize(), context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	
	// calculate buffer size
	
	// raygen sbt
	
	_raygenSBTAlignedSize += _raygenSBTStride;
	_raygenSBTAlignedSize = VKHelper::alignUp(_raygenSBTAlignedSize, context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	// triangle hit sbt
	
	for (const std::vector<std::vector<std::byte>>& recordGroup : _info.getTriangleHitRecords())
	{
		_triangleHitSBTAlignedSize += _triangleHitSBTStride * recordGroup.size();
	}
	_triangleHitSBTAlignedSize = VKHelper::alignUp(_triangleHitSBTAlignedSize, context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	// miss sbt
	
	_missSBTAlignedSize += _missSBTStride * _info.getMissRecords().size();
	_missSBTAlignedSize = VKHelper::alignUp(_missSBTAlignedSize, context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	// create buffer
	
	VKBufferInfo bufferInfo(
		_raygenSBTAlignedSize + _triangleHitSBTAlignedSize + _missSBTAlignedSize,
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	bufferInfo.setRequiredAlignment(_context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	_buffer = VKBuffer<std::byte>::create(context, bufferInfo);
	
	_raygenSBTAddress = _buffer->getDeviceAddress();
	_triangleHitSBTAddress = _raygenSBTAddress + _raygenSBTAlignedSize;
	_missSBTAddress = _triangleHitSBTAddress + _triangleHitSBTAlignedSize;
	
	// copy data to buffer
	
	// raygen sbt
	
	std::byte* raygenBufferPtr = _buffer->getHostPointer();
	
	std::memcpy(raygenBufferPtr, _info.getRaygenRecord().data(), _info.getRaygenRecord().size());
	
	// triangle hit sbt
	
	std::byte* triangleHitBufferPtr = _buffer->getHostPointer() + _raygenSBTAlignedSize;
	
	for (const std::vector<std::vector<std::byte>>& recordGroup : _info.getTriangleHitRecords())
	{
		for (const std::vector<std::byte>& record : recordGroup)
		{
			std::memcpy(triangleHitBufferPtr, record.data(), record.size());
			triangleHitBufferPtr += _triangleHitSBTStride;
		}
	}
	
	// miss sbt
	
	std::byte* missBufferPtr = _buffer->getHostPointer() + _raygenSBTAlignedSize + _triangleHitSBTAlignedSize;
	
	for (const std::vector<std::byte>& record : _info.getMissRecords())
	{
		std::memcpy(missBufferPtr, record.data(), record.size());
		missBufferPtr += _missSBTStride;
	}
}

VKShaderBindingTable::~VKShaderBindingTable()
{

}

const VKShaderBindingTableInfo& VKShaderBindingTable::getInfo() const
{
	return _info;
}

const VKPtr<VKBuffer<std::byte>>& VKShaderBindingTable::getBuffer() const
{
	return _buffer;
}

const vk::DeviceAddress& VKShaderBindingTable::getRaygenSBTAddress() const
{
	return _raygenSBTAddress;
}

const vk::DeviceSize& VKShaderBindingTable::getRaygenSBTAlignedSize() const
{
	return _raygenSBTAlignedSize;
}

const vk::DeviceSize& VKShaderBindingTable::getRaygenSBTStride() const
{
	return _raygenSBTStride;
}

const vk::DeviceAddress& VKShaderBindingTable::getTriangleHitSBTAddress() const
{
	return _triangleHitSBTAddress;
}

const vk::DeviceSize& VKShaderBindingTable::getTriangleHitSBTAlignedSize() const
{
	return _triangleHitSBTAlignedSize;
}

const vk::DeviceSize& VKShaderBindingTable::getTriangleHitSBTStride() const
{
	return _triangleHitSBTStride;
}

const vk::DeviceAddress& VKShaderBindingTable::getMissSBTAddress() const
{
	return _missSBTAddress;
}

const vk::DeviceSize& VKShaderBindingTable::getMissSBTAlignedSize() const
{
	return _missSBTAlignedSize;
}

const vk::DeviceSize& VKShaderBindingTable::getMissSBTStride() const
{
	return _missSBTStride;
}