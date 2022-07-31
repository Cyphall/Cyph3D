#pragma once

#include "Cyph3D/GLObject/Framebuffer.h"
#include "Cyph3D/GLObject/Texture.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class ShaderProgram;

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