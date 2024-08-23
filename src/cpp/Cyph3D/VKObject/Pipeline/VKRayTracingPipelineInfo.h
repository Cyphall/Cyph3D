#pragma once

#include <string>
#include <optional>

class VKPipelineLayout;

class VKRayTracingPipelineInfo
{
public:
	struct RaygenGroupInfo
	{
		std::string raygenShader;
	};

	struct TriangleHitGroupInfo
	{
		std::optional<std::string> closestHitShader;
		std::optional<std::string> anyHitShader;
	};

	struct MissGroupInfo
	{
		std::string missShader;
	};

	explicit VKRayTracingPipelineInfo(const std::shared_ptr<VKPipelineLayout>& pipelineLayout);

	const std::shared_ptr<VKPipelineLayout>& getPipelineLayout() const;

	const RaygenGroupInfo& getRaygenGroupInfo(uint32_t index) const;
	const std::vector<RaygenGroupInfo>& getRaygenGroupsInfos() const;
	void addRaygenGroupsInfos(const std::string& raygenShader);

	const TriangleHitGroupInfo& getTriangleHitGroupInfo(uint32_t index) const;
	const std::vector<TriangleHitGroupInfo>& getTriangleHitGroupsInfos() const;
	void addTriangleHitGroupsInfos(std::optional<std::string> closestHitShader, std::optional<std::string> anyHitShader);

	const MissGroupInfo& getMissGroupInfo(uint32_t index) const;
	const std::vector<MissGroupInfo>& getMissGroupsInfos() const;
	void addMissGroupsInfos(const std::string& missShader);

private:
	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::vector<RaygenGroupInfo> _raygenGroupsInfos;
	std::vector<TriangleHitGroupInfo> _triangleHitGroupsInfos;
	std::vector<MissGroupInfo> _missGroupsInfos;
};