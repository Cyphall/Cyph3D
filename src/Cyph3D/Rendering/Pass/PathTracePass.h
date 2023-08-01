#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

struct RenderRegistry;
class Camera;
template<typename T>
class VKBuffer;
template<typename T>
class VKResizableBuffer;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKRayTracingPipeline;
class VKImage;
class VKImageView;

struct PathTracePassInput
{
	const RenderRegistry& registry;
	Camera& camera;
	uint32_t sampleCount;
	bool resetAccumulation;
};

struct PathTracePassOutput
{
	const VKPtr<VKImageView>& rawRenderImageView;
	uint32_t accumulatedSamples;
};

class PathTracePass : public RenderPass<PathTracePassInput, PathTracePassOutput>
{
public:
	explicit PathTracePass(const glm::uvec2& size);

private:
	struct GlobalUniforms
	{
		GLSL_vec3 cameraPosition;
		GLSL_vec3 cameraRayTL;
		GLSL_vec3 cameraRayTR;
		GLSL_vec3 cameraRayBL;
		GLSL_vec3 cameraRayBR;
		GLSL_uint sampleIndex;
		GLSL_uint sampleCount;
		GLSL_bool resetAccumulation;
		GLSL_bool hasSkybox;
		GLSL_uint skyboxIndex;
		GLSL_mat4 skyboxRotation;
	};
	
	struct ObjectUniforms
	{
		GLSL_mat4 normalMatrix;
		GLSL_mat4 model;
		GLSL_DeviceAddress vertexBuffer;
		GLSL_DeviceAddress indexBuffer;
		GLSL_uint albedoIndex;
		GLSL_uint normalIndex;
		GLSL_uint roughnessIndex;
		GLSL_uint metalnessIndex;
		GLSL_uint displacementIndex;
		GLSL_uint emissiveIndex;
		GLSL_float emissiveScale;
	};
	
	VKDynamic<VKBuffer<GlobalUniforms>> _globalUniforms;
	VKDynamic<VKResizableBuffer<ObjectUniforms>> _objectUniforms;
	
	VKDynamic<VKResizableBuffer<std::byte>> _tlasBackingBuffer;
	VKDynamic<VKResizableBuffer<std::byte>> _tlasScratchBuffer;
	VKDynamic<VKResizableBuffer<vk::AccelerationStructureInstanceKHR>> _tlasInstancesBuffer;
	
	VKDynamic<VKResizableBuffer<std::byte>> _raygenSBT;
	VKDynamic<VKResizableBuffer<std::byte>> _missSBT;
	VKDynamic<VKResizableBuffer<std::byte>> _hitSBT;
	
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKRayTracingPipeline> _pipeline;
	
	VKPtr<VKImage> _rawRenderImage;
	VKPtr<VKImageView> _rawRenderImageView;
	
	uint32_t _sampleIndex = 0;
	uint32_t _accumulatedSamples = 0;
	
	PathTracePassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, PathTracePassInput& input) override;
	void onResize() override;
	
	void createBuffers();
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createImage();
};