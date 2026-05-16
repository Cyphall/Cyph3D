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

	std::span<const std::byte> getRaygenGroupHandle(uint32_t index) const;
	std::span<const std::byte> getTriangleHitGroupHandle(uint32_t index) const;
	std::span<const std::byte> getMissGroupHandle(uint32_t index) const;

	vk::PipelineBindPoint getPipelineType() const override;
	const std::shared_ptr<VKPipelineLayout>& getPipelineLayout() const override;

private:
	VKRayTracingPipeline(VKContext& context, const VKRayTracingPipelineInfo& info);

	VKRayTracingPipelineInfo _info;

	size_t _raygenGroupOffset;
	size_t _triangleHitGroupOffset;
	size_t _missGroupOffset;

	std::vector<std::byte> _groupsHandles;

	std::span<const std::byte> getGroupHandle(uint32_t index) const;
};