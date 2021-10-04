#pragma once

#include "RenderPass.h"
#include "../PostProcessingEffect/PostProcessingEffect.h"

class PostProcessingPass : public RenderPass
{
public:
	PostProcessingPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size);

private:
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;
	
	std::vector<std::unique_ptr<PostProcessingEffect>> _effects;
};
