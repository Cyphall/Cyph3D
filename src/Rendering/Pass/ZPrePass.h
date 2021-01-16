#pragma once


#include "../../GLObject/ShaderProgram.h"
#include "../../GLObject/Framebuffer.h"
#include "IRenderPass.h"

class ZPrePass : public IRenderPass
{
public:
	ZPrePass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipeline() override;
	void render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipeline() override;
	
private:
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _depthTexture;
	VertexArray _vao;
};


