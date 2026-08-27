#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>

namespace c3d
{
class RenderRegistry;
class Camera;

struct PathTracePassInput
{
	const RenderRegistry& registry;
	Camera& camera;
	uint32_t sampleCount;
	bool sceneChanged;
	bool cameraChanged;
};

struct PathTracePassOutput
{
	cgpu::ImagePtr lightImage;
	uint32_t accumulatedSamples;
};

class PathTracePass : public RenderPass<PathTracePassInput, PathTracePassOutput>
{
public:
	explicit PathTracePass(const glm::uvec2& size);

private:
	cgpu::TLASPtr _tlas;

	cgpu::ComputeShaderStatePtr _computeShaderState;

	cgpu::ImagePtr _lightImage;

	uint32_t _batchIndex = 0;
	uint32_t _accumulatedSamples = 0;

	PathTracePassOutput onRender(cgpu::CommandRecorder& commandRecorder, PathTracePassInput& input) override;
	void onResize() override;

	void recreateTLAS(cgpu::CommandRecorder& commandRecorder, const PathTracePassInput& input);

	void createPipelineState();
	void createImages();
};
}
