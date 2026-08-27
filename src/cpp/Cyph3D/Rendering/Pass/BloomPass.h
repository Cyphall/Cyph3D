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

	cgpu::ImagePtr _workImage;
	cgpu::ImagePtr _outputImage;

	cgpu::SamplerPtr _downsampleSampler;
	cgpu::SamplerPtr _upsampleSampler;
	cgpu::SamplerPtr _composeSampler;

	cgpu::VertexInputStatePtr _vertexInputState;
	cgpu::PreRasterizationShaderStatePtr _preRasterizationShaderState;
	cgpu::FragmentShaderStatePtr _downsampleFragmentShaderState;
	cgpu::FragmentShaderStatePtr _upsampleFragmentShaderState;
	cgpu::FragmentShaderStatePtr _composeFragmentShaderState;
	cgpu::FragmentOutputStatePtr _downsampleComposeFragmentOutputState;
	cgpu::FragmentOutputStatePtr _upsampleFragmentOutputState;


	void downsampleAndBlur(cgpu::CommandRecorder& commandRecorder, uint32_t srcLevel, uint32_t dstLevel);
	void upsampleAndBlur(cgpu::CommandRecorder& commandRecorder, uint32_t srcLevel, uint32_t dstLevel);
	void compose(cgpu::CommandRecorder& commandRecorder, const cgpu::ImagePtr& input);

	void createPipelineStates();
	void createImages();
	void createSamplers();

	BloomPassOutput onRender(cgpu::CommandRecorder& commandRecorder, BloomPassInput& input) override;
	void onResize() override;
};
}
