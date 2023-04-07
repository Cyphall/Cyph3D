#pragma once

#include "Cyph3D/GLObject/GLVertexArray.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

struct RenderRegistry;
class Camera;

struct ShadowMapPassInput
{
	RenderRegistry& registry;
	Camera& camera;
};

struct ShadowMapPassOutput
{

};

class ShadowMapPass : public RenderPass<ShadowMapPassInput, ShadowMapPassOutput>
{
public:
	explicit ShadowMapPass(glm::uvec2 size);

private:
	GLVertexArray _vao;
	
	GLShaderProgram _directionalLightShadowMappingProgram;
	GLShaderProgram _pointLightShadowMappingProgram;
	
	ShadowMapPassOutput renderImpl(ShadowMapPassInput& input) override;
};