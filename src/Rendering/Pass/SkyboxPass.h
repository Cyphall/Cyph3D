#pragma once

#include "IRenderPass.h"

class SkyboxPass : public IRenderPass
{
public:
	SkyboxPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipeline() override;
	void render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipeline() override;

private:
	Framebuffer _framebuffer;
	ShaderProgram* _shader;
	
	VertexArray _vao;
	VertexBuffer<float> _vbo;
};
