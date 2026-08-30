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
	cgpu::ComputeShaderStatePtr _computeShaderState;

	cgpu::ImagePtr _outputImage;

	void createPipelineState();
	void createImage();

	ToneMappingPassOutput onRender(cgpu::CommandRecorder& commandRecorder, ToneMappingPassInput& input) override;
	void onResize() override;
};
}
