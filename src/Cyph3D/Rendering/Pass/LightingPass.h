#pragma once

#include "Cyph3D/GLObject/GLFramebuffer.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/GLObject/GLMutableBuffer.h"

class LightingPass : public RenderPass
{
public:
	LightingPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep) override;
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
	
	GLMutableBuffer<GLSL_PointLight> _pointLightsBuffer;
	GLMutableBuffer<GLSL_DirectionalLight> _directionalLightsBuffer;
	
	GLShaderProgram _shader;
	GLFramebuffer _framebuffer;
	GLTexture _rawRenderTexture;
};