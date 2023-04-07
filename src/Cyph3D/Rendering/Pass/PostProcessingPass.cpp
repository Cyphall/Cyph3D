#include "PostProcessingPass.h"

#include "Cyph3D/Rendering/PostProcessingEffect/BloomEffect.h"
#include "Cyph3D/Rendering/PostProcessingEffect/ExposureEffect.h"
#include "Cyph3D/Rendering/PostProcessingEffect/ToneMappingEffect.h"

PostProcessingPass::PostProcessingPass(glm::uvec2 size):
RenderPass(size, "Post-processing pass")
{
	_effects.push_back(std::make_unique<ExposureEffect>(size));
	_effects.push_back(std::make_unique<BloomEffect>(size));
	_effects.push_back(std::make_unique<ToneMappingEffect>(size));
}

PostProcessingPass::~PostProcessingPass()
{}

PostProcessingPassOutput PostProcessingPass::renderImpl(PostProcessingPassInput& input)
{
	glEnable(GL_CULL_FACE);
	
	GLTexture* renderTexture = &input.rawRender;
	
	for (std::unique_ptr<PostProcessingEffect>& effect : _effects)
	{
		renderTexture = &effect->render(*renderTexture, input.camera, _renderPassPerf);
	}
	
	glDisable(GL_CULL_FACE);
	
	return PostProcessingPassOutput{
		.postProcessedRender = *renderTexture
	};
}