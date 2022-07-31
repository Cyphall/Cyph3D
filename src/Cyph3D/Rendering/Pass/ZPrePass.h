#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/GLObject/VertexArray.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class GLShaderProgram;

class ZPrePass : public RenderPass
{
public:
	ZPrePass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;
	
private:
	GLShaderProgram* _shader;
	GLFramebuffer _framebuffer;
	GLTexture _depthTexture;
	VertexArray _vao;
};