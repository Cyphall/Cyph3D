#pragma once

#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/GLObject/Framebuffer.h"
#include "Cyph3D/GLObject/VertexArray.h"

class ShaderProgram;

class SkyboxPass : public RenderPass
{
public:
	SkyboxPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep) override;
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