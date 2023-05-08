#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <filesystem>
#include <optional>

class VKPipelineLayout;

class VKRayTracingPipelineInfo
{
public:
	struct ObjectTypeInfo
	{
		std::optional<std::filesystem::path> closestHitShader;
		std::optional<std::filesystem::path> anyHitShader;
		std::optional<std::filesystem::path> intersectionShader;
	};
	
	struct RayTypeInfo
	{
		std::filesystem::path missShader;
		std::vector<ObjectTypeInfo> objectTypesInfos;
	};
	
	struct RaygenGroupInfo
	{
		std::filesystem::path raygenShader;
	};
	
	explicit VKRayTracingPipelineInfo(
		const VKPtr<VKPipelineLayout>& pipelineLayout,
		const std::filesystem::path& raygenShader);
	
	const VKPtr<VKPipelineLayout>& getPipelineLayout() const;
	
	const std::filesystem::path& getRaygenShader() const;
	
	const RayTypeInfo& getRayTypeInfo(uint32_t index) const;
	const std::vector<RayTypeInfo>& getRayTypesInfos() const;
	void addRayType(const std::filesystem::path& missShader);
	void addObjectTypeForRayType(uint32_t rayTypeIndex, std::optional<std::filesystem::path> closestHitShader, std::optional<std::filesystem::path> anyHitShader, std::optional<std::filesystem::path> intersectionShader);

private:
	VKPtr<VKPipelineLayout> _pipelineLayout;
	std::filesystem::path _raygenShader;
	std::vector<RayTypeInfo> _rayTypesInfos;
};