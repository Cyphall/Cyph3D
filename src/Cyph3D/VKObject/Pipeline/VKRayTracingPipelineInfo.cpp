#include "VKRayTracingPipelineInfo.h"

VKRayTracingPipelineInfo::VKRayTracingPipelineInfo(
	const VKPtr<VKPipelineLayout>& pipelineLayout,
	const std::filesystem::path& raygenShader):
	_pipelineLayout(pipelineLayout),
	_raygenShader(raygenShader)
{

}

const VKPtr<VKPipelineLayout>& VKRayTracingPipelineInfo::getPipelineLayout() const
{
	return _pipelineLayout;
}

const std::filesystem::path& VKRayTracingPipelineInfo::getRaygenShader() const
{
	return _raygenShader;
}

const VKRayTracingPipelineInfo::RayTypeInfo& VKRayTracingPipelineInfo::getRayTypeInfo(uint32_t index) const
{
	return _rayTypesInfos[index];
}

const std::vector<VKRayTracingPipelineInfo::RayTypeInfo>& VKRayTracingPipelineInfo::getRayTypesInfos() const
{
	return _rayTypesInfos;
}

void VKRayTracingPipelineInfo::addRayType(const std::filesystem::path& missShader)
{
	VKRayTracingPipelineInfo::RayTypeInfo& rayTypeInfo = _rayTypesInfos.emplace_back();
	rayTypeInfo.missShader = missShader;
}

void VKRayTracingPipelineInfo::addObjectTypeForRayType(uint32_t rayTypeIndex, std::optional<std::filesystem::path> closestHitShader, std::optional<std::filesystem::path> anyHitShader, std::optional<std::filesystem::path> intersectionShader)
{
	VKRayTracingPipelineInfo::ObjectTypeInfo& objectTypeInfo = _rayTypesInfos[rayTypeIndex].objectTypesInfos.emplace_back();
	objectTypeInfo.closestHitShader = std::move(closestHitShader);
	objectTypeInfo.anyHitShader = std::move(anyHitShader);
	objectTypeInfo.intersectionShader = std::move(intersectionShader);
}