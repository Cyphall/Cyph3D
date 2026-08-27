#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>

namespace c3d
{
class RenderRegistry;
class Camera;

struct ZPrepassInput
{
	const RenderRegistry& registry;
	Camera& camera;
};

struct ZPrepassOutput
{
	const cgpu::ImagePtr& multisampledDepthImage;
};

class ZPrepass : public RenderPass<ZPrepassInput, ZPrepassOutput>
{
public:
	explicit ZPrepass(glm::uvec2 size);

private:
	cgpu::ImagePtr _multisampledDepthImage;

	cgpu::VertexInputStatePtr _vertexInputState;
	cgpu::PreRasterizationShaderStatePtr _preRasterizationShaderState;
	cgpu::FragmentShaderStatePtr _fragmentShaderState;
	cgpu::FragmentOutputStatePtr _fragmentOutputState;

	ZPrepassOutput onRender(cgpu::CommandRecorder& commandRecorder, ZPrepassInput& input) override;
	void onResize() override;

	void createImage();
	void createPipelineStates();
};
}
