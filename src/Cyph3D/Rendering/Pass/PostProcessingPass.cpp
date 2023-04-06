#include "PostProcessingPass.h"

#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/Rendering/PostProcessingEffect/BloomEffect.h"
#include "Cyph3D/Rendering/PostProcessingEffect/ExposureEffect.h"
#include "Cyph3D/Rendering/PostProcessingEffect/ToneMappingEffect.h"

PostProcessingPass::PostProcessingPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size):
RenderPass(textures, size, "Post-processing pass")
{
	_effects.push_back(std::make_unique<ExposureEffect>(size));
	_effects.push_back(std::make_unique<BloomEffect>(size));
	_effects.push_back(std::make_unique<ToneMappingEffect>(size));
}

PostProcessingPass::~PostProcessingPass()
{}

void PostProcessingPass::preparePipelineImpl()
{
	glEnable(GL_CULL_FACE);
}

void PostProcessingPass::renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep)
{
	GLTexture* renderTexture = textures["raw_render"];
	
	glm::ivec2 size = getSize();
	
	for (std::unique_ptr<PostProcessingEffect>& effect : _effects)
	{
		glViewport(0, 0, size.x, size.y);
		auto [outputTexture, effectPerf] = effect->render(renderTexture, textures, camera);
		renderTexture = outputTexture;
		previousFramePerfStep.addSubstep(effectPerf);
	}
	
	textures["final"] = renderTexture;
}

void PostProcessingPass::restorePipelineImpl()
{
	glDisable(GL_CULL_FACE);
}