#pragma once

#include "Cyph3D/Rendering/Pass/RenderPass.h"

#include <memory>

class PostProcessingEffect;
class GLTexture;
class Camera;

struct PostProcessingPassInput
{
	GLTexture& rawRender;
	Camera& camera;
};

struct PostProcessingPassOutput
{
	GLTexture& postProcessedRender;
};

class PostProcessingPass : public RenderPass<PostProcessingPassInput, PostProcessingPassOutput>
{
public:
	explicit PostProcessingPass(glm::uvec2 size);
	~PostProcessingPass() override;

private:
	std::vector<std::unique_ptr<PostProcessingEffect>> _effects;
	
	PostProcessingPassOutput renderImpl(PostProcessingPassInput& input) override;
};