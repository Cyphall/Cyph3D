#include "VKAccelerationStructure.h"

#include "Cyph3D/VKObject/Buffer/VKBuffer.h"

VKPtr<VKAccelerationStructure> VKAccelerationStructure::create(
	VKContext& context,
	vk::AccelerationStructureTypeKHR type,
	vk::DeviceSize size,
	const VKPtr<VKBufferBase>& backingBuffer)
{
	return VKPtr<VKAccelerationStructure>(new VKAccelerationStructure(context, type, size, backingBuffer));
}

VKAccelerationStructure::VKAccelerationStructure(
	VKContext& context,
	vk::AccelerationStructureTypeKHR type,
	vk::DeviceSize size,
	const VKPtr<VKBufferBase>& backingBuffer):
	VKObject(context), _type(type)
{
	if (!_context.isRayTracingSupported())
	{
		throw;
	}
	
	vk::AccelerationStructureCreateInfoKHR accelerationStructureCreateInfo;
	accelerationStructureCreateInfo.createFlags = {};
	accelerationStructureCreateInfo.buffer = backingBuffer->getHandle();
	accelerationStructureCreateInfo.offset = 0;
	accelerationStructureCreateInfo.size = size;
	accelerationStructureCreateInfo.type = type;
	accelerationStructureCreateInfo.deviceAddress = 0;
	
	_accelerationStructure = _context.getDevice().createAccelerationStructureKHR(accelerationStructureCreateInfo);
	
	_accelerationStructureAddress = _context.getDevice().getAccelerationStructureAddressKHR(_accelerationStructure);
	
	_accelerationStructureBackingBuffer = backingBuffer;
}

VKAccelerationStructure::~VKAccelerationStructure()
{
	if (_accelerationStructure)
	{
		_context.getDevice().destroyAccelerationStructureKHR(_accelerationStructure);
	}
}

const vk::AccelerationStructureKHR& VKAccelerationStructure::getHandle()
{
	return _accelerationStructure;
}

vk::AccelerationStructureTypeKHR VKAccelerationStructure::getType() const
{
	return _type;
}

vk::DeviceAddress VKAccelerationStructure::getDeviceAddress() const
{
	return _accelerationStructureAddress;
}

vk::AccelerationStructureBuildSizesInfoKHR VKAccelerationStructure::getBottomLevelBuildSizesInfo(VKContext& context, const VKBottomLevelAccelerationStructureBuildInfo& buildInfo)
{
	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
	geometry.geometry.triangles = vk::AccelerationStructureGeometryTrianglesDataKHR();
	geometry.geometry.triangles.vertexFormat = buildInfo.vertexFormat;
	geometry.geometry.triangles.vertexData = VK_NULL_HANDLE;
	geometry.geometry.triangles.vertexStride = buildInfo.vertexStride;
	geometry.geometry.triangles.maxVertex = buildInfo.vertexBuffer->getSize() - 1;
	geometry.geometry.triangles.indexType = buildInfo.indexType;
	geometry.geometry.triangles.indexData = VK_NULL_HANDLE;
	geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
	
	uint32_t primitiveCount = buildInfo.indexBuffer->getSize() / 3;
	
	vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
	buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	buildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;
	buildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	buildGeometryInfo.dstAccelerationStructure = VK_NULL_HANDLE;
	buildGeometryInfo.geometryCount = 1;
	buildGeometryInfo.pGeometries = &geometry;
	buildGeometryInfo.scratchData = VK_NULL_HANDLE;
	
	return context.getDevice().getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		buildGeometryInfo,
		primitiveCount);
}

vk::AccelerationStructureBuildSizesInfoKHR VKAccelerationStructure::getTopLevelBuildSizesInfo(VKContext& context, const VKTopLevelAccelerationStructureBuildInfo& buildInfo)
{
	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eInstances;
	geometry.geometry.instances = vk::AccelerationStructureGeometryInstancesDataKHR();
	geometry.geometry.instances.arrayOfPointers = false;
	geometry.geometry.instances.data = VK_NULL_HANDLE;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
	
	uint32_t primitiveCount = buildInfo.instancesInfos.size();
	
	vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
	buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	buildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild;
	buildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	buildGeometryInfo.dstAccelerationStructure = VK_NULL_HANDLE;
	buildGeometryInfo.geometryCount = 1;
	buildGeometryInfo.pGeometries = &geometry;
	buildGeometryInfo.scratchData = VK_NULL_HANDLE;
	
	return context.getDevice().getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		buildGeometryInfo,
		primitiveCount);
}