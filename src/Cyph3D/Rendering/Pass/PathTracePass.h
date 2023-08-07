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
class VKDescriptorSet;
class VKPipelineLayout;
class VKRayTracingPipeline;
class VKImage;
class VKImageView;

struct PathTracePassInput
{
	const RenderRegistry& registry;
	Camera& camera;
	uint32_t sampleCount;
	bool sceneChanged;
	bool cameraChanged;
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
	struct CameraUniforms
	{
		GLSL_vec3 cameraPosition;
		GLSL_vec3 cameraRayTL;
		GLSL_vec3 cameraRayTR;
		GLSL_vec3 cameraRayBL;
		GLSL_vec3 cameraRayBR;
	};
	
	struct SkyboxUniforms
	{
		GLSL_bool hasSkybox;
		GLSL_uint skyboxIndex;
		GLSL_mat4 skyboxRotation;
	};
	
	struct ObjectUniforms
	{
		GLSL_mat4 normalMatrix;
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
	
	struct FramePushConstants
	{
		GLSL_uint batchIndex;
		GLSL_uint sampleCount;
		GLSL_bool resetAccumulation;
	};
	
	VKPtr<VKAccelerationStructure> _tlas;
	
	VKPtr<VKBuffer<CameraUniforms>> _cameraUniformsBuffer;
	VKPtr<VKBuffer<SkyboxUniforms>> _skyboxUniformsBuffer;
	VKPtr<VKBuffer<ObjectUniforms>> _objectUniformsBuffer;
	
	VKDynamic<VKResizableBuffer<std::byte>> _raygenSBT;
	VKDynamic<VKResizableBuffer<std::byte>> _missSBT;
	VKDynamic<VKResizableBuffer<std::byte>> _hitSBT;
	
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	
	VKPtr<VKDescriptorSet> _descriptorSet;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKRayTracingPipeline> _pipeline;
	
	VKPtr<VKImage> _rawRenderImage;
	VKPtr<VKImageView> _rawRenderImageView;
	
	uint32_t _batchIndex = 0;
	uint32_t _accumulatedSamples = 0;
	
	PathTracePassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, PathTracePassInput& input) override;
	void onResize() override;
	
	void setupTLAS(const VKPtr<VKCommandBuffer>& commandBuffer, const PathTracePassInput& input);
	void setupCameraUniformsBuffer(const PathTracePassInput& input);
	void setupSkyboxUniformsBuffer();
	void setupObjectUniformsBuffer(const PathTracePassInput& input);
	
	void createBuffers();
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createImage();
};