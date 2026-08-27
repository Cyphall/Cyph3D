#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>

namespace c3d
{
class Camera;

struct ToneMappingPassInput
{
	const cgpu::ImagePtr& lightImage;
};

struct ToneMappingPassOutput
{
	const cgpu::ImagePtr& colorImage;
};

class ToneMappingPass : public RenderPass<ToneMappingPassInput, ToneMappingPassOutput>
{
public:
	explicit ToneMappingPass(glm::uvec2 size);

private:
	cgpu::VertexInputStatePtr _vertexInputState;
	cgpu::PreRasterizationShaderStatePtr _preRasterizationShaderState;
	cgpu::FragmentShaderStatePtr _fragmentShaderState;
	cgpu::FragmentOutputStatePtr _fragmentOutputState;

	cgpu::ImagePtr _outputImage;

	void createPipelineStates();
	void createImage();

	ToneMappingPassOutput onRender(cgpu::CommandRecorder& commandRecorder, ToneMappingPassInput& input) override;
	void onResize() override;
};
}
