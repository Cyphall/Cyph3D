#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/GLImmutableBuffer.h"
#include "Cyph3D/GLObject/GLVertexArray.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class Camera;

struct SkyboxPassInput
{
	Camera& camera;
	GLTexture& rawRender;
	GLTexture& depth;
};

struct SkyboxPassOutput
{

};

class SkyboxPass : public RenderPass<SkyboxPassInput, SkyboxPassOutput>
{
public:
	explicit SkyboxPass(glm::uvec2 size);

private:
	struct VertexData
	{
		glm::vec3 position;
	};
	
	GLFramebuffer _framebuffer;
	GLShaderProgram _shader;
	
	GLVertexArray _vao;
	GLImmutableBuffer<VertexData> _vbo;
	
	SkyboxPassOutput renderImpl(SkyboxPassInput& input) override;
};