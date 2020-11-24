#include "PostProcessingPass.h"
#include "../ToneMappingPostProcess.h"

PostProcessingPass::PostProcessingPass(std::unordered_map<std::string, Texture*>& textures):
IRenderPass(textures)
{
	_effects.push_back(std::make_unique<ToneMappingPostProcess>());
}

void PostProcessingPass::preparePipeline()
{
	glEnable(GL_CULL_FACE);
}

void PostProcessingPass::render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
{
	Texture* renderTexture = textures["raw_render"];
	
	for (int i = 0; i < _effects.size(); i++)
	{
		renderTexture = _effects[i]->render(renderTexture, textures);
	}
	
	textures["final"] = renderTexture;
}

void PostProcessingPass::restorePipeline()
{
	glDisable(GL_CULL_FACE);
}
