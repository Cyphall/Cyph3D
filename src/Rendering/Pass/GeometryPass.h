#pragma once

#include "IRenderPass.h"
#include "../../GLObject/Framebuffer.h"

class GeometryPass : public IRenderPass
{
public:
	GeometryPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipeline() override;
	void render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipeline() override;

private:
	Framebuffer _gbuffer;
	
	Texture _normalTexture;
	Texture _colorTexture;
	Texture _materialTexture;
	Texture _geometryNormalTexture;
	Texture _depthTexture;
	Texture _objectIndexTexture;
	
	VertexArray _vao;
};
