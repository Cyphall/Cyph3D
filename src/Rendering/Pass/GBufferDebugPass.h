#pragma once

#include "RenderPass.h"

class GBufferDebugPass : public RenderPass
{
public:
	GBufferDebugPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipelineImpl() override;

private:
	
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _debugTexture;
};


