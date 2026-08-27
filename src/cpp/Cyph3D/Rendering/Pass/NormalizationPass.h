#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>

namespace c3d
{
struct NormalizationPassInput
{
	cgpu::ImagePtr lightImage;
	uint32_t accumulatedSamples;
};

struct NormalizationPassOutput
{
	const cgpu::ImagePtr& lightImage;
};

class NormalizationPass : public RenderPass<NormalizationPassInput, NormalizationPassOutput>
{
public:
	explicit NormalizationPass(glm::uvec2 size);

private:
	cgpu::ComputeShaderStatePtr _computeShaderState;

	cgpu::ImagePtr _outputImage;

	void createPipelineState();
	void createImage();

	NormalizationPassOutput onRender(cgpu::CommandRecorder& commandRecorder, NormalizationPassInput& input) override;
	void onResize() override;
};
}
