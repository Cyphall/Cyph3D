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

void PostProcessingPass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep)
{
	Texture* renderTexture = textures["raw_render"];
	
	glm::ivec2 size = getSize();
	
	for (auto& effect : _effects)
	{
		glViewport(0, 0, size.x, size.y);
		auto pair = effect->render(renderTexture, textures, camera);
		renderTexture = pair.first;
		previousFramePerfStep.subSteps.push_back(pair.second);
	}
	
	textures["final"] = renderTexture;
}

void PostProcessingPass::restorePipelineImpl()
{
	glDisable(GL_CULL_FACE);
}
