#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/Rendering/Pass/ShadowMapPass.h"
#include "Cyph3D/VKObject/VKDynamic.h"

class RenderRegistry;
class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKDescriptorSetLayout;
class VKImage;
template<typename T>
class VKResizableBuffer;

struct LightingPassInput
{
	const std::shared_ptr<VKImage>& multisampledDepthImage;
	const RenderRegistry& registry;
	Camera& camera;
	const std::vector<DirectionalShadowMapInfo>& directionalShadowMapInfos;
	const std::vector<PointShadowMapInfo>& pointShadowMapInfos;
	float pointLightMaxDistance;
};

struct LightingPassOutput
{
	const std::shared_ptr<VKImage>& multisampledRawRenderImage;
};

class LightingPass : public RenderPass<LightingPassInput, LightingPassOutput>
{
public:
	explicit LightingPass(glm::uvec2 size);

private:
	struct DirectionalLightUniforms
	{
		GLSL_vec3 fragToLightDirection;
		GLSL_float intensity;
		GLSL_vec3 color;
		GLSL_bool castShadows;
		GLSL_mat4 lightViewProjection;
		GLSL_uint textureIndex;
		GLSL_float shadowMapTexelWorldSize;
	};

	struct PointLightUniforms
	{
		GLSL_vec3 pos;
		GLSL_float intensity;
		GLSL_vec3 color;
		GLSL_bool castShadows;
		GLSL_uint textureIndex;
		GLSL_float maxTexelSizeAtUnitDistance;
	};

	struct ObjectUniforms
	{
		GLSL_mat4 normalMatrix;
		GLSL_mat4 model;
		GLSL_mat4 mvp;
		GLSL_int albedoIndex;
		GLSL_int normalIndex;
		GLSL_int roughnessIndex;
		GLSL_int metalnessIndex;
		GLSL_int displacementIndex;
		GLSL_int emissiveIndex;
		GLSL_vec3 albedoValue;
		GLSL_float roughnessValue;
		GLSL_float metalnessValue;
		GLSL_float displacementScale;
		GLSL_float emissiveScale;
	};

	struct PushConstantData
	{
		GLSL_vec3 viewPos;
		GLSL_uint frameIndex;
		GLSL_int directionalLightCount;
		GLSL_int pointLightCount;
		GLSL_float pointLightMaxDistance;
	};

	VKDynamic<VKResizableBuffer<DirectionalLightUniforms>> _directionalLightsUniforms;
	VKDynamic<VKResizableBuffer<PointLightUniforms>> _pointLightsUniforms;

	VKDynamic<VKResizableBuffer<ObjectUniforms>> _objectUniforms;

	std::shared_ptr<VKSampler> _directionalLightSampler;
	std::shared_ptr<VKSampler> _pointLightSampler;

	std::shared_ptr<VKDescriptorSetLayout> _directionalLightDescriptorSetLayout;
	VKDynamic<VKDescriptorSet> _directionalLightDescriptorSet;
	std::shared_ptr<VKDescriptorSetLayout> _pointLightDescriptorSetLayout;
	VKDynamic<VKDescriptorSet> _pointLightDescriptorSet;

	std::shared_ptr<VKDescriptorSetLayout> _objectDescriptorSetLayout;

	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _pipeline;

	std::shared_ptr<VKImage> _multisampledRawRenderImage;

	uint32_t _frameIndex = 0;

	LightingPassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, LightingPassInput& input) override;
	void onResize() override;

	void createUniformBuffers();
	void createSamplers();
	void createDescriptorSetLayouts();
	void createPipelineLayout();
	void createPipeline();
	void createImage();

	void descriptorSetsResizeSmart(uint32_t directionalLightShadowsCount, uint32_t pointLightShadowsCount);
};