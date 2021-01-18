#pragma once

#include "IRenderPass.h"

class GBufferDebugPass : public IRenderPass
{
public:
	GBufferDebugPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipeline() override;
	void render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipeline() override;

private:
	
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _debugTexture;
};


