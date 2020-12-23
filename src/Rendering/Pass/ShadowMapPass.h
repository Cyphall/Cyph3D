#pragma once

#include "IRenderPass.h"

class ShadowMapPass : public IRenderPass
{
public:
	ShadowMapPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipeline() override;
	void render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipeline() override;

private:
	VertexArray _vao;
};
