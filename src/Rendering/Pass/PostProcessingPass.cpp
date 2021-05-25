#include "PostProcessingPass.h"
#include "../PostProcessingEffect/ToneMappingEffect.h"
#include "../PostProcessingEffect/ExposureEffect.h"
#include "../PostProcessingEffect/BloomEffect.h"

PostProcessingPass::PostProcessingPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size):
RenderPass(textures, size, "Post-processing pass")
{
	_effects.push_back(std::make_unique<ExposureEffect>(size));
	_effects.push_back(std::make_unique<BloomEffect>(size));
	_effects.push_back(std::make_unique<ToneMappingEffect>(size));
}

void PostProcessingPass::preparePipelineImpl()
{
	glEnable(GL_CULL_FACE);
}

void PostProcessingPass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera)
{
	Texture* renderTexture = textures["raw_render"];
	
	for (auto& effect : _effects)
	{
		renderTexture = effect->render(renderTexture, textures);
	}
	
	textures["final"] = renderTexture;
}

void PostProcessingPass::restorePipelineImpl()
{
	glDisable(GL_CULL_FACE);
}
