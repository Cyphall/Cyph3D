#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>
#include <vector>

class VKAccelerationStructure;

struct VKTopLevelAccelerationStructureBuildInfo
{
	struct InstanceInfo
	{
		glm::mat4 localToWorld;
		uint32_t customIndex;
		uint32_t recordIndex;
		VKPtr<VKAccelerationStructure> accelerationStructure;
	};

	std::vector<InstanceInfo> instancesInfos;
};