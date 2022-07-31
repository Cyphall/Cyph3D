#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/VertexArray.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class GLShaderProgram;

class SkyboxPass : public RenderPass
{
public:
	SkyboxPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;

private:
	struct VertexData
	{
		glm::vec3 position;
	};
	
	GLFramebuffer _framebuffer;
	GLShaderProgram* _shader;
	
	VertexArray _vao;
	Buffer<VertexData> _vbo;
};