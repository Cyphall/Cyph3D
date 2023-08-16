#include "VKRayTracingPipelineInfo.h"

VKRayTracingPipelineInfo::VKRayTracingPipelineInfo(const VKPtr<VKPipelineLayout>& pipelineLayout):
	_pipelineLayout(pipelineLayout)
{

}

const VKPtr<VKPipelineLayout>& VKRayTracingPipelineInfo::getPipelineLayout() const
{
	return _pipelineLayout;
}

const VKRayTracingPipelineInfo::RaygenGroupInfo& VKRayTracingPipelineInfo::getRaygenGroupInfo(uint32_t index) const
{
	return _raygenGroupsInfos[index];
}

const std::vector<VKRayTracingPipelineInfo::RaygenGroupInfo>& VKRayTracingPipelineInfo::getRaygenGroupsInfos() const
{
	return _raygenGroupsInfos;
}

void VKRayTracingPipelineInfo::addRaygenGroupsInfos(const std::filesystem::path& raygenShader)
{
	RaygenGroupInfo& info = _raygenGroupsInfos.emplace_back();
	info.raygenShader = raygenShader;
}

const VKRayTracingPipelineInfo::TriangleHitGroupInfo& VKRayTracingPipelineInfo::getTriangleHitGroupInfo(uint32_t index) const
{
	return _triangleHitGroupsInfos[index];
}

const std::vector<VKRayTracingPipelineInfo::TriangleHitGroupInfo>& VKRayTracingPipelineInfo::getTriangleHitGroupsInfos() const
{
	return _triangleHitGroupsInfos;
}

void VKRayTracingPipelineInfo::addTriangleHitGroupsInfos(std::optional<std::filesystem::path> closestHitShader, std::optional<std::filesystem::path> anyHitShader)
{
	TriangleHitGroupInfo& info = _triangleHitGroupsInfos.emplace_back();
	info.closestHitShader = std::move(closestHitShader);
	info.anyHitShader = std::move(anyHitShader);
}

const VKRayTracingPipelineInfo::MissGroupInfo& VKRayTracingPipelineInfo::getMissGroupInfo(uint32_t index) const
{
	return _missGroupsInfos[index];
}

const std::vector<VKRayTracingPipelineInfo::MissGroupInfo>& VKRayTracingPipelineInfo::getMissGroupsInfos() const
{
	return _missGroupsInfos;
}

void VKRayTracingPipelineInfo::addMissGroupsInfos(const std::filesystem::path& missShader)
{
	MissGroupInfo& info = _missGroupsInfos.emplace_back();
	info.missShader = missShader;
}