#include "VKRayTracingPipelineInfo.h"

c3d::VKRayTracingPipelineInfo::VKRayTracingPipelineInfo(const std::shared_ptr<VKPipelineLayout>& pipelineLayout):
	_pipelineLayout(pipelineLayout)
{
}

const std::shared_ptr<c3d::VKPipelineLayout>& c3d::VKRayTracingPipelineInfo::getPipelineLayout() const
{
	return _pipelineLayout;
}

const c3d::VKRayTracingPipelineInfo::RaygenGroupInfo& c3d::VKRayTracingPipelineInfo::getRaygenGroupInfo(uint32_t index) const
{
	return _raygenGroupsInfos[index];
}

const std::vector<c3d::VKRayTracingPipelineInfo::RaygenGroupInfo>& c3d::VKRayTracingPipelineInfo::getRaygenGroupsInfos() const
{
	return _raygenGroupsInfos;
}

void c3d::VKRayTracingPipelineInfo::addRaygenGroupsInfos(const std::string& raygenShader)
{
	RaygenGroupInfo& info = _raygenGroupsInfos.emplace_back();
	info.raygenShader = raygenShader;
}

const c3d::VKRayTracingPipelineInfo::TriangleHitGroupInfo& c3d::VKRayTracingPipelineInfo::getTriangleHitGroupInfo(uint32_t index) const
{
	return _triangleHitGroupsInfos[index];
}

const std::vector<c3d::VKRayTracingPipelineInfo::TriangleHitGroupInfo>& c3d::VKRayTracingPipelineInfo::getTriangleHitGroupsInfos() const
{
	return _triangleHitGroupsInfos;
}

void c3d::VKRayTracingPipelineInfo::addTriangleHitGroupsInfos(std::optional<std::string> closestHitShader, std::optional<std::string> anyHitShader)
{
	TriangleHitGroupInfo& info = _triangleHitGroupsInfos.emplace_back();
	info.closestHitShader = std::move(closestHitShader);
	info.anyHitShader = std::move(anyHitShader);
}

const c3d::VKRayTracingPipelineInfo::MissGroupInfo& c3d::VKRayTracingPipelineInfo::getMissGroupInfo(uint32_t index) const
{
	return _missGroupsInfos[index];
}

const std::vector<c3d::VKRayTracingPipelineInfo::MissGroupInfo>& c3d::VKRayTracingPipelineInfo::getMissGroupsInfos() const
{
	return _missGroupsInfos;
}

void c3d::VKRayTracingPipelineInfo::addMissGroupsInfos(const std::string& missShader)
{
	MissGroupInfo& info = _missGroupsInfos.emplace_back();
	info.missShader = missShader;
}