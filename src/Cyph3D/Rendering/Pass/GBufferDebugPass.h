#pragma once

#include "RenderPass.h"

class GBufferDebugPass : public RenderPass
{
public:
	GBufferDebugPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;

private:
	
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _debugTexture;
};


