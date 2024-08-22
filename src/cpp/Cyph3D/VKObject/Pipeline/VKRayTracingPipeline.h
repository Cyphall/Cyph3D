#pragma once

#include "Cyph3D/VKObject/Pipeline/VKPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKRayTracingPipelineInfo.h"

#include <array>
#include <vulkan/vulkan.hpp>

class VKRayTracingPipeline : public VKPipeline
{
public:
	static std::shared_ptr<VKRayTracingPipeline> create(VKContext& context, const VKRayTracingPipelineInfo& info);

	~VKRayTracingPipeline() override;

	const VKRayTracingPipelineInfo& getInfo() const;

	const std::array<std::byte, 32>& getRaygenGroupHandle(uint32_t index) const;
	size_t getRaygenGroupCount() const;

	const std::array<std::byte, 32>& getTriangleHitGroupHandle(uint32_t index) const;
	size_t getTriangleHitGroupCount() const;

	const std::array<std::byte, 32>& getMissGroupHandle(uint32_t index) const;
	size_t getMissGroupCount() const;

	vk::PipelineBindPoint getPipelineType() const override;
	const std::shared_ptr<VKPipelineLayout>& getPipelineLayout() const override;

private:
	VKRayTracingPipeline(VKContext& context, const VKRayTracingPipelineInfo& info);

	VKRayTracingPipelineInfo _info;

	std::vector<std::array<std::byte, 32>> _raygenGroupsHandles;
	std::vector<std::array<std::byte, 32>> _triangleHitGroupsHandles;
	std::vector<std::array<std::byte, 32>> _missGroupsHandles;
};