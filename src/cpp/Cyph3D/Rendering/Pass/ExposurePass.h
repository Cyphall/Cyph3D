#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>

namespace c3d
{
class Camera;

struct ExposurePassInput
{
	const cgpu::ImagePtr& lightImage;
	const Camera& camera;
};

struct ExposurePassOutput
{
	const cgpu::ImagePtr& lightImage;
};

class ExposurePass : public RenderPass<ExposurePassInput, ExposurePassOutput>
{
public:
	explicit ExposurePass(glm::uvec2 size);

private:
	cgpu::ComputeShaderStatePtr _computeShaderState;

	void createPipelineState();

	ExposurePassOutput onRender(cgpu::CommandRecorder& commandRecorder, ExposurePassInput& input) override;
};
}
