#pragma once


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
		std::shared_ptr<VKAccelerationStructure> accelerationStructure;
	};

	std::vector<InstanceInfo> instancesInfos;
};