#pragma once

#include "IRenderPass.h"
#include "../IPostProcessEffect.h"

class PostProcessingPass : public IRenderPass
{
public:
	PostProcessingPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipeline() override;
	void render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipeline() override;
	
private:
	std::vector<std::unique_ptr<IPostProcessEffect>> _effects;
};