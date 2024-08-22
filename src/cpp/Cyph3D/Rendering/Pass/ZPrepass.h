#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

struct RenderRegistry;
class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;

struct ZPrepassInput
{
	const RenderRegistry& registry;
	Camera& camera;
};

struct ZPrepassOutput
{
	const std::shared_ptr<VKImage>& multisampledDepthImage;
};

class ZPrepass : public RenderPass<ZPrepassInput, ZPrepassOutput>
{
public:
	explicit ZPrepass(glm::uvec2 size);

private:
	struct PushConstantData
	{
		GLSL_mat4 mvp;
	};

	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _pipeline;

	std::shared_ptr<VKImage> _depthImage;

	ZPrepassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, ZPrepassInput& input) override;
	void onResize() override;

	void createPipelineLayout();
	void createPipeline();
	void createImage();
};