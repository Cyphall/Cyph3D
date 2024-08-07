#pragma once

#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/VKObject/VKPtr.h"

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
	const VKPtr<VKImage>& multisampledDepthImage;
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

	VKPtr<VKPipelineLayout> _pipelineLayout;
	VKPtr<VKGraphicsPipeline> _pipeline;

	VKPtr<VKImage> _depthImage;

	ZPrepassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ZPrepassInput& input) override;
	void onResize() override;

	void createPipelineLayout();
	void createPipeline();
	void createImage();
};