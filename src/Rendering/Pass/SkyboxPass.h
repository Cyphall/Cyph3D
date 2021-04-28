#pragma once

#include "RenderPass.h"

class SkyboxPass : public RenderPass
{
public:
	SkyboxPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipelineImpl() override;

private:
	struct VertexData
	{
		glm::vec3 position;
	};
	
	Framebuffer _framebuffer;
	ShaderProgram* _shader;
	
	VertexArray _vao;
	Buffer<VertexData> _vbo;
};
