#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

struct RenderRegistry;
class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;
class VKImageView;
template<typename T>
class VKResizableBuffer;

struct LightingPassInput
{
	const VKPtr<VKImageView>& depthImageView;
	RenderRegistry& registry;
	Camera& camera;
};

struct LightingPassOutput
{
	const VKPtr<VKImageView>& rawRenderImageView;
	const VKPtr<VKImageView>& objectIndexImageView;
};

class LightingPass : public RenderPass<LightingPassInput, LightingPassOutput>
{
public:
	explicit LightingPass(glm::uvec2 size);

private:
	struct DirectionalLightUniforms
	{
		GLSL_vec3  fragToLightDirection;
		GLSL_float intensity;
		GLSL_vec3  color;
		GLSL_bool  castShadows;
		GLSL_mat4  lightViewProjection;
		GLSL_uint  textureIndex;
		GLSL_float shadowMapTexelWorldSize;
	};
	
	struct PointLightUniforms
	{
		GLSL_vec3  pos;
		GLSL_float intensity;
		GLSL_vec3  color;
		GLSL_bool  castShadows;
		GLSL_uint  textureIndex;
		GLSL_float far;
		GLSL_float maxTexelSizeAtUnitDistance;
	};
	
	struct ObjectUniforms
	{
		GLSL_mat4 normalMatrix;
		GLSL_mat4 model;
		GLSL_mat4 mvp;
		GLSL_int  objectIndex;
		GLSL_uint albedoIndex;
		GLSL_uint normalIndex;
		GLSL_uint roughnessIndex;
		GLSL_uint metalnessIndex;
		GLSL_uint displacementIndex;
		GLSL_uint emissiveIndex;
		GLSL_float emissiveScale;
	};
	
	struct PushConstantData
	{
		GLSL_mat4 viewProjectionInv;
		GLSL_vec3 viewPos;
		GLSL_uint frameIndex;
	};
	
	VKDynamic<VKResizableBuffer<DirectionalLightUniforms>> _directionalLightsUniforms;
	VKDynamic<VKResizableBuffer<PointLightUniforms>> _pointLightsUniforms;
	
	VKDynamic<VKResizableBuffer<ObjectUniforms>> _objectUniforms;
	
	VKPtr<VKSampler> _directionalLightSampler;
	VKPtr<VKSampler> _pointLightSampler;
	
	VKPtr<VKDescriptorSetLayout> _directionalLightDescriptorSetLayout;
	VKDynamic<VKDescriptorSet> _directionalLightDescriptorSet;
	VKPtr<VKDescriptorSetLayout> _pointLightDescriptorSetLayout;
	VKDynamic<VKDescriptorSet> _pointLightDescriptorSet;
	
	VKPtr<VKDescriptorSetLayout> _objectDescriptorSetLayout;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKGraphicsPipeline> _pipeline;
	
	VKDynamic<VKImage> _rawRenderImage;
	VKDynamic<VKImageView> _rawRenderImageView;
	VKDynamic<VKImage> _objectIndexImage;
	VKDynamic<VKImageView> _objectIndexImageView;
	
	uint32_t _frameIndex = 0;
	
	LightingPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, LightingPassInput& input) override;
	void onResize() override;
	
	void createUniformBuffers();
	void createSamplers();
	void createDescriptorSetLayouts();
	void createPipelineLayout();
	void createPipeline();
	void createImages();
	
	void descriptorSetsResizeSmart(uint32_t directionalLightShadowsCount, uint32_t pointLightShadowsCount);
};