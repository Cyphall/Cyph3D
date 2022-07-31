#pragma once

#include "Cyph3D/GLObject/GLVertexArray.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class GLShaderProgram;

class ShadowMapPass : public RenderPass
{
public:
	ShadowMapPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;

private:
	GLVertexArray _vao;
	
	GLShaderProgram* _directionalLightShadowMappingProgram;
	GLShaderProgram* _pointLightShadowMappingProgram;
};