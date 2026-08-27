#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>
#include <Cyph3D/Rendering/RenderRegistry.h>
#include <Cyph3D/Rendering/ShadowMapManager.h>

namespace c3d
{
struct DirectionalShadowMapInfo
{
	float worldSize;
	float worldDepth;
	glm::mat4 vpMatrix;
	cgpu::ImagePtr image;
};

struct PointShadowMapInfo
{
	cgpu::ImagePtr image;
};

struct ShadowMapPassInput
{
	const RenderRegistry& registry;
	bool sceneChanged;
	bool cameraChanged;
};

struct ShadowMapPassOutput
{
	const std::vector<std::optional<DirectionalShadowMapInfo>>& directionalShadowMapInfos;
	const std::vector<std::optional<PointShadowMapInfo>>& pointShadowMapInfos;
	float pointLightMaxDistance;
};

class ShadowMapPass : public RenderPass<ShadowMapPassInput, ShadowMapPassOutput>
{
public:
	explicit ShadowMapPass(glm::uvec2 size);

private:
	ShadowMapManager _shadowMapManager;

	cgpu::VertexInputStatePtr _vertexInputState;

	cgpu::PreRasterizationShaderStatePtr _directionalPreRasterizationShaderState;
	cgpu::FragmentShaderStatePtr _directionalFragmentShaderState;
	cgpu::FragmentOutputStatePtr _directionalFragmentOutputState;

	cgpu::PreRasterizationShaderStatePtr _pointPreRasterizationShaderState;
	cgpu::FragmentShaderStatePtr _pointFragmentShaderState;
	cgpu::FragmentOutputStatePtr _pointFragmentOutputState;

	std::vector<std::optional<DirectionalShadowMapInfo>> _directionalShadowMapInfos;
	std::vector<std::optional<PointShadowMapInfo>> _pointShadowMapInfos;

	ShadowMapPassOutput onRender(cgpu::CommandRecorder& commandRecorder, ShadowMapPassInput& input) override;
	void onResize() override;

	void createPipelineStates();

	DirectionalShadowMapInfo renderDirectionalShadowMap(
		cgpu::CommandRecorder& commandRecorder,
		const DirectionalLight::RenderData& light,
		const std::vector<ModelRenderer::RenderData>& models
	);

	PointShadowMapInfo renderPointShadowMap(
		cgpu::CommandRecorder& commandRecorder,
		const PointLight::RenderData& light,
		const std::vector<ModelRenderer::RenderData>& models
	);
};
}
