#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

struct RenderRegistry;
class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKDescriptorSetLayout;
template<typename T>
class VKResizableBuffer;

struct ShadowMapPassInput
{
	const RenderRegistry& registry;
	Camera& camera;
	bool sceneChanged;
	bool cameraChanged;
};

struct ShadowMapPassOutput
{

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
	
	VKPtr<VKPipelineLayout> _directionalLightPipelineLayout;
	VKPtr<VKGraphicsPipeline> _directionalLightPipeline;
	
	VKPtr<VKDescriptorSetLayout> _pointLightDescriptorSetLayout;
	VKDynamic<VKResizableBuffer<PointLightUniforms>> _pointLightUniformBuffer;
	VKPtr<VKPipelineLayout> _pointLightPipelineLayout;
	VKPtr<VKGraphicsPipeline> _pointLightPipeline;
	
	ShadowMapPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ShadowMapPassInput& input) override;
	void onResize() override;
	
	void createDescriptorSetLayout();
	void createBuffer();
	void createPipelineLayouts();
	void createPipelines();
};