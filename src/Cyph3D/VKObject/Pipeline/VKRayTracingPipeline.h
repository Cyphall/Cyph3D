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
	
	const std::array<std::byte, 32>& getRaygenGroupHandle() const;
	
	const std::array<std::byte, 32>& getMissGroupHandle(uint32_t rayTypeIndex) const;
	size_t getMissGroupCount() const;
	
	const std::array<std::byte, 32>& getHitGroupHandle(uint32_t rayTypeIndex, uint32_t objectTypeIndex) const;
	size_t getHitGroupCount(uint32_t rayTypeIndex) const;
	size_t getHitGroupCount() const;
	
	vk::PipelineBindPoint getPipelineType() const override;
	const VKPtr<VKPipelineLayout>& getPipelineLayout() const override;

private:
	struct RayType
	{
		std::array<std::byte, 32> missGroupHandle;
		std::vector<std::array<std::byte, 32>> objectsTypesHandles;
	};
	
	VKRayTracingPipeline(VKContext& context, VKRayTracingPipelineInfo& info);
	
	VKRayTracingPipelineInfo _info;
	
	std::array<std::byte, 32> _raygenGroupHandle;
	std::vector<RayType> _rayTypes;
	size_t _totalHitGroupCount = 0;
};