#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>

namespace c3d
{
class Camera;

struct BloomPassInput
{
	const cgpu::ImagePtr& lightImage;
};

struct BloomPassOutput
{
	const cgpu::ImagePtr& lightImage;
};

class BloomPass : public RenderPass<BloomPassInput, BloomPassOutput>
{
public:
	explicit BloomPass(glm::uvec2 size);

private:
	// common

	std::vector<cgpu::ImagePtr> _workImages;

	cgpu::SamplerPtr _downsampleSampler;
	cgpu::SamplerPtr _upsampleSampler;

	cgpu::ComputeShaderStatePtr _downsampleShaderState;
	cgpu::ComputeShaderStatePtr _upsampleShaderState;


	void downsample(cgpu::CommandRecorder& commandRecorder, uint32_t srcLevel, uint32_t dstLevel);
	void upsample(cgpu::CommandRecorder& commandRecorder, uint32_t srcLevel, uint32_t dstLevel);

	void createPipelineStates();
	void createImages();
	void createSamplers();

	BloomPassOutput onRender(cgpu::CommandRecorder& commandRecorder, BloomPassInput& input) override;
	void onResize() override;
};
}
