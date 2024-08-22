#pragma once

#include "Cyph3D/VKObject/AccelerationStructure/VKBottomLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKTopLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>

class VKBufferBase;

class VKAccelerationStructure : public VKObject
{
public:
	static std::shared_ptr<VKAccelerationStructure> create(
		VKContext& context,
		vk::AccelerationStructureTypeKHR type,
		vk::DeviceSize size
	);

	~VKAccelerationStructure() override;

	const vk::AccelerationStructureKHR& getHandle();

	vk::AccelerationStructureTypeKHR getType() const;

	vk::DeviceAddress getDeviceAddress() const;

	std::shared_ptr<VKBufferBase> getBackingBuffer();

	static vk::AccelerationStructureBuildSizesInfoKHR getBottomLevelBuildSizesInfo(
		VKContext& context,
		const VKBottomLevelAccelerationStructureBuildInfo& buildInfo
	);

	static vk::AccelerationStructureBuildSizesInfoKHR getTopLevelBuildSizesInfo(
		VKContext& context,
		const VKTopLevelAccelerationStructureBuildInfo& buildInfo
	);

protected:
	friend class VKCommandBuffer;

	VKAccelerationStructure(
		VKContext& context,
		vk::AccelerationStructureTypeKHR type,
		vk::DeviceSize size
	);

	vk::AccelerationStructureTypeKHR _type;
	std::shared_ptr<VKBufferBase> _backingBuffer;
	vk::AccelerationStructureKHR _handle = VK_NULL_HANDLE;
	vk::DeviceAddress _deviceAddress = 0;

	std::vector<std::shared_ptr<VKObject>> _referencedObjectsInBuild;
};