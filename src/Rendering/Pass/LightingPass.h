#pragma once

#include "IRenderPass.h"
#include "../../GLObject/ShaderStorageBuffer.h"

class LightingPass : public IRenderPass
{
public:
	LightingPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipeline() override;
	void render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipeline() override;
	
private:
	ShaderStorageBuffer<PointLight::LightData> _pointLightsBuffer;
	ShaderStorageBuffer<DirectionalLight::LightData> _directionalLightsBuffer;
	
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _rawRenderTexture;
};
