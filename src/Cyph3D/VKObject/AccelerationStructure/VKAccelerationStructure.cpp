#include "VKAccelerationStructure.h"

#include "Cyph3D/VKObject/Buffer/VKBuffer.h"

VKPtr<VKAccelerationStructure> VKAccelerationStructure::create(
	VKContext& context,
	vk::AccelerationStructureTypeKHR type,
	vk::DeviceSize size
)
{
	return VKPtr<VKAccelerationStructure>(new VKAccelerationStructure(context, type, size));
}

VKAccelerationStructure::VKAccelerationStructure(
	VKContext& context,
	vk::AccelerationStructureTypeKHR type,
	vk::DeviceSize size
):
	VKObject(context),
	_type(type)
{
	if (!_context.isRayTracingSupported())
	{
		throw;
	}

	VKBufferInfo backingBufferInfo(size, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);
	backingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);

	_backingBuffer = VKBuffer<std::byte>::create(context, backingBufferInfo);

	vk::AccelerationStructureCreateInfoKHR accelerationStructureCreateInfo;
	accelerationStructureCreateInfo.createFlags = {};
	accelerationStructureCreateInfo.buffer = _backingBuffer->getHandle();
	accelerationStructureCreateInfo.offset = 0;
	accelerationStructureCreateInfo.size = size;
	accelerationStructureCreateInfo.type = type;
	accelerationStructureCreateInfo.deviceAddress = 0;

	_handle = _context.getDevice().createAccelerationStructureKHR(accelerationStructureCreateInfo);

	vk::AccelerationStructureDeviceAddressInfoKHR accelerationStructureDeviceAddressInfo;
	accelerationStructureDeviceAddressInfo.accelerationStructure = _handle;

	_deviceAddress = _context.getDevice().getAccelerationStructureAddressKHR(accelerationStructureDeviceAddressInfo);
}

VKAccelerationStructure::~VKAccelerationStructure()
{
	_context.getDevice().destroyAccelerationStructureKHR(_handle);
}

const vk::AccelerationStructureKHR& VKAccelerationStructure::getHandle()
{
	return _handle;
}

vk::AccelerationStructureTypeKHR VKAccelerationStructure::getType() const
{
	return _type;
}

vk::DeviceAddress VKAccelerationStructure::getDeviceAddress() const
{
	return _deviceAddress;
}

VKPtr<VKBufferBase> VKAccelerationStructure::getBackingBuffer()
{
	return _backingBuffer;
}

vk::AccelerationStructureBuildSizesInfoKHR VKAccelerationStructure::getBottomLevelBuildSizesInfo(VKContext& context, const VKBottomLevelAccelerationStructureBuildInfo& buildInfo)
{
	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
	geometry.geometry.triangles = vk::AccelerationStructureGeometryTrianglesDataKHR();
	geometry.geometry.triangles.vertexFormat = buildInfo.vertexFormat;
	geometry.geometry.triangles.vertexData = VK_NULL_HANDLE;
	geometry.geometry.triangles.vertexStride = buildInfo.vertexStride;
	geometry.geometry.triangles.maxVertex = buildInfo.vertexBuffer->getInfo().getSize() - 1;
	geometry.geometry.triangles.indexType = buildInfo.indexType;
	geometry.geometry.triangles.indexData = VK_NULL_HANDLE;
	geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;

	uint32_t primitiveCount = buildInfo.indexBuffer->getInfo().getSize() / 3;

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
		primitiveCount
	);
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
	buildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	buildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	buildGeometryInfo.dstAccelerationStructure = VK_NULL_HANDLE;
	buildGeometryInfo.geometryCount = 1;
	buildGeometryInfo.pGeometries = &geometry;
	buildGeometryInfo.scratchData = VK_NULL_HANDLE;

	return context.getDevice().getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice,
		buildGeometryInfo,
		primitiveCount
	);
}