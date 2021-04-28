#pragma once

#include "RenderPass.h"
#include "../../GLObject/Framebuffer.h"

class GeometryPass : public RenderPass
{
public:
	GeometryPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
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
};
