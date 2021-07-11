#pragma once

#include "RenderPass.h"
#include "../../GLObject/ShaderStorageBuffer.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera) override;
	void restorePipelineImpl() override;
	
private:
	struct GLSLPointLight
	{
		glm::vec3  pos;
		float32_t  intensity;
		glm::vec3  color;
		int32_t    castShadows; // bool
		uint64_t   shadowMap;
		float32_t  far;
		float32_t  maxTexelSizeAtUnitDistance;
	};
	
	struct GLSLDirectionalLight
	{
		glm::vec3  fragToLightDirection;
		float32_t  intensity;
		glm::vec3  color;
		int32_t    castShadows; // 32-bit bool
		glm::mat4  lightViewProjection;
		uint64_t   shadowMap;
		float32_t  mapSize;
		float32_t  mapDepth;
	};
	
	ShaderStorageBuffer<GLSLPointLight> _pointLightsBuffer;
	ShaderStorageBuffer<GLSLDirectionalLight> _directionalLightsBuffer;
	
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _rawRenderTexture;
};
