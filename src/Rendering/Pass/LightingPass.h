#pragma once

#include "RenderPass.h"
#include "../../GLObject/ShaderStorageBuffer.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) override;
	void restorePipelineImpl() override;
	
private:
	ShaderStorageBuffer<PointLight::LightData> _pointLightsBuffer;
	ShaderStorageBuffer<DirectionalLight::LightData> _directionalLightsBuffer;
	
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _rawRenderTexture;
};
