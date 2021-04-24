#include "PostProcessingPass.h"
#include "../PostProcessingEffect/ToneMappingEffect.h"
#include "../PostProcessingEffect/ExposureEffect.h"
#include "../PostProcessingEffect/BloomEffect.h"

PostProcessingPass::PostProcessingPass(std::unordered_map<std::string, Texture*>& textures):
IRenderPass(textures)
{
	_effects.push_back(std::make_unique<ExposureEffect>());
	_effects.push_back(std::make_unique<BloomEffect>());
	_effects.push_back(std::make_unique<ToneMappingEffect>());
}

void PostProcessingPass::preparePipeline()
{
	glEnable(GL_CULL_FACE);
}

void PostProcessingPass::render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
{
	Texture* renderTexture = textures["raw_render"];
	
	for (auto& effect : _effects)
	{
		renderTexture = effect->render(renderTexture, textures);
	}
	
	textures["final"] = renderTexture;
}

void PostProcessingPass::restorePipeline()
{
	glDisable(GL_CULL_FACE);
}
