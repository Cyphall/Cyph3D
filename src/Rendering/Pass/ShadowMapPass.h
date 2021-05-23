#pragma once

#include "RenderPass.h"

class ShadowMapPass : public RenderPass
{
public:
	ShadowMapPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera) override;
	void restorePipelineImpl() override;

private:
	VertexArray _vao;
};
