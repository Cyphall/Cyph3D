#pragma once

#include "Cyph3D/Rendering/Pass/RenderPass.h"

#include <memory>

class PostProcessingEffect;

class PostProcessingPass : public RenderPass
{
public:
	PostProcessingPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size);

private:
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;
	
	std::vector<std::unique_ptr<PostProcessingEffect>> _effects;
};