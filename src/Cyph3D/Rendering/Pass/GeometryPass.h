#pragma once

#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/GLObject/Framebuffer.h"

class GeometryPass : public RenderPass
{
public:
	GeometryPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;

private:
	Framebuffer _gbuffer;
	
	Texture _normalTexture;
	Texture _positionTexture;
	Texture _colorTexture;
	Texture _materialTexture;
	Texture _geometryNormalTexture;
	Texture _objectIndexTexture;
	
	VertexArray _vao;
	
	ShaderProgram* _shaderProgram;
};