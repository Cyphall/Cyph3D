#include "PostProcessingPass.h"

#include "Cyph3D/Rendering/PostProcessingEffect/PostProcessingEffect.h"
#include "Cyph3D/Rendering/PostProcessingEffect/BloomEffect.h"
#include "Cyph3D/Rendering/PostProcessingEffect/ExposureEffect.h"
#include "Cyph3D/Rendering/PostProcessingEffect/ToneMappingEffect.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"

PostProcessingPass::PostProcessingPass(glm::uvec2 size):
RenderPass(size, "Post-processing pass")
{
	_effects.push_back(std::make_unique<ExposureEffect>(size));
	_effects.push_back(std::make_unique<BloomEffect>(size));
	_effects.push_back(std::make_unique<ToneMappingEffect>(size));
}

PostProcessingPass::~PostProcessingPass()
{}

PostProcessingPassOutput PostProcessingPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, PostProcessingPassInput& input)
{
	const VKPtr<VKImageView>* renderImageView = &input.rawRenderImageView;
	
	for (std::unique_ptr<PostProcessingEffect>& effect : _effects)
	{
		renderImageView = &effect->render(commandBuffer, *renderImageView, input.camera, _renderPassPerf);
	}
	
	return PostProcessingPassOutput{
		.postProcessedRenderImageView = *renderImageView
	};
}

void PostProcessingPass::onResize()
{
	for (std::unique_ptr<PostProcessingEffect>& effect : _effects)
	{
		effect->resize(_size);
	}
}