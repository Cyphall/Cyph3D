#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/GLObject/GLVertexArray.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"

class GeometryPass : public RenderPass
{
public:
	GeometryPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;

private:
	GLFramebuffer _gbuffer;
	
	GLTexture _normalTexture;
	GLTexture _positionTexture;
	GLTexture _colorTexture;
	GLTexture _materialTexture;
	GLTexture _geometryNormalTexture;
	GLTexture _objectIndexTexture;
	
	GLVertexArray _vao;
	
	GLShaderProgram _shaderProgram;
};