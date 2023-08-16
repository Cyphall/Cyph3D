#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

#include <filesystem>
#include <optional>

class VKPipelineLayout;

class VKRayTracingPipelineInfo
{
public:
	struct RaygenGroupInfo
	{
		std::filesystem::path raygenShader;
	};
	
	struct TriangleHitGroupInfo
	{
		std::optional<std::filesystem::path> closestHitShader;
		std::optional<std::filesystem::path> anyHitShader;
	};
	
	struct MissGroupInfo
	{
		std::filesystem::path missShader;
	};
	
	explicit VKRayTracingPipelineInfo(const VKPtr<VKPipelineLayout>& pipelineLayout);
	
	const VKPtr<VKPipelineLayout>& getPipelineLayout() const;
	
	const RaygenGroupInfo& getRaygenGroupInfo(uint32_t index) const;
	const std::vector<RaygenGroupInfo>& getRaygenGroupsInfos() const;
	void addRaygenGroupsInfos(const std::filesystem::path& raygenShader);
	
	const TriangleHitGroupInfo& getTriangleHitGroupInfo(uint32_t index) const;
	const std::vector<TriangleHitGroupInfo>& getTriangleHitGroupsInfos() const;
	void addTriangleHitGroupsInfos(std::optional<std::filesystem::path> closestHitShader, std::optional<std::filesystem::path> anyHitShader);
	
	const MissGroupInfo& getMissGroupInfo(uint32_t index) const;
	const std::vector<MissGroupInfo>& getMissGroupsInfos() const;
	void addMissGroupsInfos(const std::filesystem::path& missShader);

private:
	VKPtr<VKPipelineLayout> _pipelineLayout;
	std::vector<RaygenGroupInfo> _raygenGroupsInfos;
	std::vector<TriangleHitGroupInfo> _triangleHitGroupsInfos;
	std::vector<MissGroupInfo> _missGroupsInfos;
};