#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/GLObject/GLVertexArray.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

struct RenderRegistry;
class Camera;

struct ZPrepassInput
{
	RenderRegistry& registry;
	Camera& camera;
};

struct ZPrepassOutput
{
	GLTexture& depth;
};

class ZPrepass : public RenderPass<ZPrepassInput, ZPrepassOutput>
{
public:
	explicit ZPrepass(glm::uvec2 size);
	
private:
	GLShaderProgram _shader;
	GLFramebuffer _framebuffer;
	GLTexture _depthTexture;
	GLVertexArray _vao;
	
	ZPrepassOutput renderImpl(ZPrepassInput& input) override;
};