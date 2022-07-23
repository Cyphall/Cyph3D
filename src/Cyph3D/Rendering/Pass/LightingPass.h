#pragma once

#include "RenderPass.h"
#include "../../GLObject/ShaderStorageBuffer.h"
#include "../../GLSL_types.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;
	
private:
	struct GLSL_PointLight
	{
		GLSL_vec3  pos;
		GLSL_float intensity;
		GLSL_vec3  color;
		GLSL_bool  castShadows;
		GLSL_samplerCube shadowMap;
		GLSL_float far;
		GLSL_float maxTexelSizeAtUnitDistance;
	};
	
	struct GLSL_DirectionalLight
	{
		GLSL_vec3  fragToLightDirection;
		GLSL_float intensity;
		GLSL_vec3  color;
		GLSL_bool  castShadows;
		GLSL_mat4  lightViewProjection;
		GLSL_sampler2D shadowMap;
		GLSL_float mapSize;
		GLSL_float mapDepth;
	};
	
	ShaderStorageBuffer<GLSL_PointLight> _pointLightsBuffer;
	ShaderStorageBuffer<GLSL_DirectionalLight> _directionalLightsBuffer;
	
	ShaderProgram* _shader;
	Framebuffer _framebuffer;
	Texture _rawRenderTexture;
};
