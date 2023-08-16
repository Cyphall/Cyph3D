#pragma once

#include "Cyph3D/VKObject/Pipeline/VKPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKRayTracingPipelineInfo.h"

#include <vulkan/vulkan.hpp>
#include <array>

class VKRayTracingPipeline : public VKPipeline
{
public:
	static VKPtr<VKRayTracingPipeline> create(VKContext& context, VKRayTracingPipelineInfo& info);
	
	~VKRayTracingPipeline() override;
	
	const VKRayTracingPipelineInfo& getInfo() const;
	
	const std::array<std::byte, 32>& getRaygenGroupHandle(uint32_t index) const;
	size_t getRaygenGroupCount() const;
	
	const std::array<std::byte, 32>& getTriangleHitGroupHandle(uint32_t index) const;
	size_t getTriangleHitGroupCount() const;
	
	const std::array<std::byte, 32>& getMissGroupHandle(uint32_t index) const;
	size_t getMissGroupCount() const;
	
	vk::PipelineBindPoint getPipelineType() const override;
	const VKPtr<VKPipelineLayout>& getPipelineLayout() const override;

private:
	VKRayTracingPipeline(VKContext& context, VKRayTracingPipelineInfo& info);
	
	VKRayTracingPipelineInfo _info;
	
	std::vector<std::array<std::byte, 32>> _raygenGroupsHandles;
	std::vector<std::array<std::byte, 32>> _triangleHitGroupsHandles;
	std::vector<std::array<std::byte, 32>> _missGroupsHandles;
};