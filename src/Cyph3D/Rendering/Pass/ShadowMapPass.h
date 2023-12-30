#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/SceneRenderer/RasterizationModelData.h"
#include "Cyph3D/Rendering/ShadowMapManager.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/VKObject/VKPtr.h"

class VKPipelineLayout;
class VKGraphicsPipeline;
class VKDescriptorSetLayout;
template<typename T>
class VKBuffer;
template<typename T>
class VKResizableBuffer;

struct DirectionalShadowMapInfo
{
	float worldSize;
	float worldDepth;
	glm::mat4 viewProjection;
	VKPtr<VKImage> image;
};

struct PointShadowMapInfo
{
	VKPtr<VKImage> image;
};

struct ShadowMapPassInput
{
	const RenderRegistry& registry;
	VKPtr<VKBuffer<RasterizationModelData>> modelDataBuffer;
	VKPtr<VKBuffer<vk::DrawIndirectCommand>> drawCommandsBuffer;
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
		GLSL_mat4 viewProjection;
		GLSL_DeviceAddress modelDataBuffer;
	};

	struct PointLightPushConstantData
	{
		GLSL_mat4 viewProjection;
		GLSL_DeviceAddress modelDataBuffer;
		GLSL_vec3 lightPos;
	};

	ShadowMapManager _shadowMapManager;

	VKPtr<VKPipelineLayout> _directionalLightPipelineLayout;
	VKPtr<VKGraphicsPipeline> _directionalLightPipeline;
	std::vector<DirectionalShadowMapInfo> _directionalShadowMapInfos;

	VKPtr<VKPipelineLayout> _pointLightPipelineLayout;
	VKPtr<VKGraphicsPipeline> _pointLightPipeline;
	std::vector<PointShadowMapInfo> _pointShadowMapInfos;

	ShadowMapPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ShadowMapPassInput& input) override;
	void onResize() override;

	void createPipelineLayouts();
	void createPipelines();

	void renderDirectionalShadowMap(
		const VKPtr<VKCommandBuffer>& commandBuffer,
		const DirectionalLight::RenderData& light,
		const std::vector<ModelRenderer::RenderData>& models,
		const VKPtr<VKBuffer<RasterizationModelData>>& modelDataBuffer,
		const VKPtr<VKBuffer<vk::DrawIndirectCommand>>& drawCommandsBuffer
	);

	void renderPointShadowMap(
		const VKPtr<VKCommandBuffer>& commandBuffer,
		const PointLight::RenderData& light,
		const std::vector<ModelRenderer::RenderData>& models,
		const VKPtr<VKBuffer<RasterizationModelData>>& modelDataBuffer,
		const VKPtr<VKBuffer<vk::DrawIndirectCommand>>& drawCommandsBuffer
	);
};