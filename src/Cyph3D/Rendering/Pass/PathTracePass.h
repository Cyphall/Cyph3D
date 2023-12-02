#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/VKObject/VKPtr.h"

struct RenderRegistry;
class Camera;
class VKDescriptorSetLayout;
class VKDescriptorSet;
class VKPipelineLayout;
class VKRayTracingPipeline;
class VKImage;
class VKShaderBindingTable;

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
	std::array<VKPtr<VKImage>, 3> rawRenderImage;
	uint32_t accumulatedSamples;
	uint32_t fixedPointDecimals;
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

	struct CubemapSkyboxUniforms
	{
		GLSL_uint skyboxIndex;
		GLSL_mat4 skyboxRotation;
	};

	struct ObjectUniforms
	{
		GLSL_mat4 normalMatrix;
		GLSL_DeviceAddress vertexBuffer;
		GLSL_DeviceAddress indexBuffer;
		GLSL_int albedoIndex;
		GLSL_int normalIndex;
		GLSL_int roughnessIndex;
		GLSL_int metalnessIndex;
		GLSL_int displacementIndex;
		GLSL_int emissiveIndex;
		GLSL_vec3 albedoValue;
		GLSL_float roughnessValue;
		GLSL_float metalnessValue;
		GLSL_float emissiveScale;
	};

	struct FramePushConstants
	{
		GLSL_uint batchIndex;
		GLSL_uint sampleCount;
		GLSL_bool resetAccumulation;
		GLSL_uint fixedPointDecimals;
	};

	VKPtr<VKAccelerationStructure> _tlas;

	VKPtr<VKShaderBindingTable> _sbt;

	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;

	VKPtr<VKDescriptorSet> _descriptorSet;

	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKRayTracingPipeline> _pipeline;

	std::array<VKPtr<VKImage>, 3> _rawRenderImage;

	uint32_t _batchIndex = 0;
	uint32_t _accumulatedSamples = 0;

	PathTracePassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, PathTracePassInput& input) override;
	void onResize() override;

	void setupTLAS(const VKPtr<VKCommandBuffer>& commandBuffer, const PathTracePassInput& input);
	void setupSBT(const PathTracePassInput& input);

	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createImage();
};