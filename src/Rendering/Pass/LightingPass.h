#pragma once

#include "RenderPass.h"
#include "../../GLObject/ShaderStorageBuffer.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(std::unordered_map<std::string, Texture*>& textures);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera) override;
	void restorePipelineImpl() override;
	
private:
	ShaderStorageBuffer<PointLight::NativeData> _pointLightsBuffer;
	ShaderStorageBuffer<DirectionalLight::NativeData> _directionalLightsBuffer;
	
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _rawRenderTexture;
};
