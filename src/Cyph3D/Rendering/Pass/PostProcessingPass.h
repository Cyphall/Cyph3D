#pragma once

#include "Cyph3D/Rendering/Pass/RenderPass.h"

class PostProcessingEffect;
class Camera;
class VKImageView;

struct PostProcessingPassInput
{
	const VKPtr<VKImageView>& rawRenderView;
	Camera& camera;
};

struct PostProcessingPassOutput
{
	const VKPtr<VKImageView>& postProcessedRenderView;
};

class PostProcessingPass : public RenderPass<PostProcessingPassInput, PostProcessingPassOutput>
{
public:
	explicit PostProcessingPass(glm::uvec2 size);
	~PostProcessingPass() override;

private:
	std::vector<std::unique_ptr<PostProcessingEffect>> _effects;
	
	PostProcessingPassOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, PostProcessingPassInput& input) override;
};