#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>

namespace c3d
{
class Camera;

struct SkyboxPassInput
{
	Camera& camera;
	const cgpu::ImagePtr& multisampledLightImage;
	const cgpu::ImagePtr& multisampledDepthImage;
};

struct SkyboxPassOutput
{
	const cgpu::ImagePtr& lightImage;
};

class SkyboxPass : public RenderPass<SkyboxPassInput, SkyboxPassOutput>
{
public:
	explicit SkyboxPass(glm::uvec2 size);

private:
	cgpu::VertexInputStatePtr _vertexInputState;
	cgpu::PreRasterizationShaderStatePtr _preRasterizationShaderState;
	cgpu::FragmentShaderStatePtr _fragmentShaderState;
	cgpu::FragmentOutputStatePtr _fragmentOutputState;

	cgpu::ImagePtr _lightImage;

	cgpu::BufferPtr _vertexBuffer;

	SkyboxPassOutput onRender(cgpu::CommandRecorder& commandRecorder, SkyboxPassInput& input) override;
	void onResize() override;

	void createPipelineStates();
	void createImages();
	void createBuffer();
};
}
