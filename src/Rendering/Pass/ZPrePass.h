#pragma once


#include "../../GLObject/ShaderProgram.h"
#include "../../GLObject/Framebuffer.h"
#include "RenderPass.h"

class ZPrePass : public RenderPass
{
public:
	ZPrePass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera) override;
	void restorePipelineImpl() override;
	
private:
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _depthTexture;
	VertexArray _vao;
};


