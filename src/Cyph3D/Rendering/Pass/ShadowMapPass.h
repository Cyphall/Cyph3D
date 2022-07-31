#pragma once

#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/GLObject/VertexArray.h"

class ShaderProgram;

class ShadowMapPass : public RenderPass
{
public:
	ShadowMapPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;

private:
	VertexArray _vao;
	
	ShaderProgram* _directionalLightShadowMappingProgram;
	ShaderProgram* _pointLightShadowMappingProgram;
};