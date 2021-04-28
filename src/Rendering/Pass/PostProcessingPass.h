#pragma once

#include "RenderPass.h"
#include "../PostProcessingEffect/PostProcessingEffect.h"

class PostProcessingPass : public RenderPass
{
public:
	PostProcessingPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipelineImpl() override;
	
private:
	std::vector<std::unique_ptr<PostProcessingEffect>> _effects;
};
