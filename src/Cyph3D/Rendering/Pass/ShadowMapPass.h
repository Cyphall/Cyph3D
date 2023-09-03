#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/ShadowMapManager.h"

class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKDescriptorSetLayout;
template<typename T>
class VKResizableBuffer;

struct DirectionalShadowMapInfo
{
	float worldSize;
	float worldDepth;
	glm::mat4 viewProjection;
	VKPtr<VKImageView> imageView;
};

struct PointShadowMapInfo
{
	VKPtr<VKImageView> imageView;
};

struct ShadowMapPassInput
{
	const RenderRegistry& registry;
	Camera& camera;
	bool sceneChanged;
	bool cameraChanged;
};

struct ShadowMapPassOutput
{
	const std::vector<DirectionalShadowMapInfo>& directionalShadowMapInfos;
	const std::vector<PointShadowMapInfo>& pointShadowMapInfos;
};

class ShadowMapPass : public RenderPass<ShadowMapPassInput, ShadowMapPassOutput>
{
public:
	explicit ShadowMapPass(glm::uvec2 size);

private:
	struct DirectionalLightPushConstantData
	{
		GLSL_mat4 mvp;
	};
	
	struct PointLightUniforms
	{
		GLSL_mat4 viewProjections[6];
		GLSL_vec3 lightPos;
	};
	
	struct PointLightPushConstantData
	{
		GLSL_mat4 model;
	};
	
	ShadowMapManager _shadowMapManager;
	
	VKPtr<VKPipelineLayout> _directionalLightPipelineLayout;
	VKPtr<VKGraphicsPipeline> _directionalLightPipeline;
	std::vector<DirectionalShadowMapInfo> _directionalShadowMapInfos;
	
	VKPtr<VKDescriptorSetLayout> _pointLightDescriptorSetLayout;
	VKDynamic<VKResizableBuffer<PointLightUniforms>> _pointLightUniformBuffer;
	VKPtr<VKPipelineLayout> _pointLightPipelineLayout;
	VKPtr<VKGraphicsPipeline> _pointLightPipeline;
	std::vector<PointShadowMapInfo> _pointShadowMapInfos;
	
	ShadowMapPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ShadowMapPassInput& input) override;
	void onResize() override;
	
	void createDescriptorSetLayout();
	void createBuffer();
	void createPipelineLayouts();
	void createPipelines();
	
	void renderDirectionalShadowMap(
		const VKPtr<VKCommandBuffer>& commandBuffer,
		const DirectionalLight::RenderData& light,
		const std::vector<ModelRenderer::RenderData>& models,
		const Camera& camera);
	
	void renderPointShadowMap(
		const VKPtr<VKCommandBuffer>& commandBuffer,
		const PointLight::RenderData& light,
		const std::vector<ModelRenderer::RenderData>& models,
		int uniformIndex);
};