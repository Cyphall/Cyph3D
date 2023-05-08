#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

struct RenderRegistry;
class Camera;
template<typename T>
class VKResizableBuffer;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKRayTracingPipeline;
class VKImage;
class VKImageView;

struct RaytracePassInput
{
	RenderRegistry& registry;
	Camera& camera;
};

struct RaytracePassOutput
{
	const VKPtr<VKImageView>& rawRenderImageView;
	const VKPtr<VKImageView>& objectIndexImageView;
};

class RaytracePass : public RenderPass<RaytracePassInput, RaytracePassOutput>
{
public:
	explicit RaytracePass(const glm::uvec2& size);

private:
	struct PushConstantData
	{
		GLSL_vec3 position;
		GLSL_vec3 rayTL;
		GLSL_vec3 rayTR;
		GLSL_vec3 rayBL;
		GLSL_vec3 rayBR;
	};
	
	VKDynamic<VKResizableBuffer<std::byte>> _tlasBackingBuffer;
	VKDynamic<VKResizableBuffer<std::byte>> _tlasScratchBuffer;
	VKDynamic<VKResizableBuffer<vk::AccelerationStructureInstanceKHR>> _tlasInstancesBuffer;
	
	VKDynamic<VKResizableBuffer<std::byte>> _raygenSBT;
	VKDynamic<VKResizableBuffer<std::byte>> _missSBT;
	VKDynamic<VKResizableBuffer<std::byte>> _hitSBT;
	
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKRayTracingPipeline> _pipeline;
	
	VKDynamic<VKImage> _rawRenderImage;
	VKDynamic<VKImageView> _rawRenderImageView;
	VKDynamic<VKImage> _objectIndexImage;
	VKDynamic<VKImageView> _objectIndexImageView;
	
	RaytracePassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, RaytracePassInput& input) override;
	void onResize() override;
	
	void createBuffers();
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createImages();
};