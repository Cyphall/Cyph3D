#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>
#include <Cyph3D/Rendering/Pass/ShadowMapPass.h>

namespace c3d
{
class RenderRegistry;
class Camera;

struct LightingPassInput
{
	const cgpu::ImagePtr& multisampledDepthImage;
	const RenderRegistry& registry;
	Camera& camera;
	const std::vector<std::optional<DirectionalShadowMapInfo>>& directionalShadowMapInfos;
	const std::vector<std::optional<PointShadowMapInfo>>& pointShadowMapInfos;
	float pointLightMaxDistance;
};

struct LightingPassOutput
{
	const cgpu::ImagePtr& lightImage;
};

class LightingPass : public RenderPass<LightingPassInput, LightingPassOutput>
{
public:
	explicit LightingPass(glm::uvec2 size);

private:
	cgpu::ImagePtr _multisampledLightImage;
	cgpu::ImagePtr _lightImage;

	cgpu::SamplerPtr _directionalLightSampler;
	cgpu::SamplerPtr _pointLightSampler;

	cgpu::VertexInputStatePtr _vertexInputState;

	cgpu::PreRasterizationShaderStatePtr _objectPreRasterizationShaderState;
	cgpu::FragmentShaderStatePtr _objectFragmentShaderState;

	cgpu::PreRasterizationShaderStatePtr _skyboxPreRasterizationShaderState;
	cgpu::FragmentShaderStatePtr _skyboxFragmentShaderState;

	cgpu::FragmentOutputStatePtr _fragmentOutputState;

	uint32_t _frameIndex = 0;

	LightingPassOutput onRender(cgpu::CommandRecorder& commandRecorder, LightingPassInput& input) override;
	void onResize() override;

	void createImage();
	void createSamplers();
	void createPipelineStates();
};
}
