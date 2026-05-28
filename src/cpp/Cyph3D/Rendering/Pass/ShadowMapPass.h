#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>
#include <Cyph3D/Rendering/RenderRegistry.h>
#include <Cyph3D/Rendering/ShadowMapManager.h>
#include <Cyph3D/VKObject/VKDynamic.h>

namespace c3d
{
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
	std::shared_ptr<VKImage> image;
};

struct PointShadowMapInfo
{
	std::shared_ptr<VKImage> image;
};

struct ShadowMapPassInput
{
	const RenderRegistry& registry;
	bool sceneChanged;
	bool cameraChanged;
};

struct ShadowMapPassOutput
{
	const std::vector<DirectionalShadowMapInfo>& directionalShadowMapInfos;
	const std::vector<PointShadowMapInfo>& pointShadowMapInfos;
	float pointLightMaxDistance;
};

class ShadowMapPass : public RenderPass<ShadowMapPassInput, ShadowMapPassOutput>
{
public:
	explicit ShadowMapPass(glm::uvec2 size);

private:
	struct DirectionalLightPushConstantData
	{
		glm::mat4 mvp;
	};

	//FIXME: properly align storage buffer offset
	struct alignas(16) PointLightUniforms
	{
		glm::mat4 viewProjection;
		glm::vec3 lightPos;
		float maxDistance;
	};

	struct PointLightPushConstantData
	{
		glm::mat4 model;
	};

	ShadowMapManager _shadowMapManager;

	std::shared_ptr<VKPipelineLayout> _directionalLightPipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _directionalLightPipeline;
	std::vector<DirectionalShadowMapInfo> _directionalShadowMapInfos;

	std::shared_ptr<VKDescriptorSetLayout> _pointLightDescriptorSetLayout;
	VKDynamic<VKResizableBuffer<PointLightUniforms>> _pointLightUniformBuffer;
	std::shared_ptr<VKPipelineLayout> _pointLightPipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _pointLightPipeline;
	std::vector<PointShadowMapInfo> _pointShadowMapInfos;

	ShadowMapPassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, ShadowMapPassInput& input) override;
	void onResize() override;

	void createDescriptorSetLayout();
	void createBuffer();
	void createPipelineLayouts();
	void createPipelines();

	void renderDirectionalShadowMap(
		const std::shared_ptr<VKCommandBuffer>& commandBuffer,
		const DirectionalLight::RenderData& light,
		const std::vector<ModelRenderer::RenderData>& models
	);

	void renderPointShadowMap(
		const std::shared_ptr<VKCommandBuffer>& commandBuffer,
		const PointLight::RenderData& light,
		const std::vector<ModelRenderer::RenderData>& models,
		int uniformIndex
	);
};
}