#pragma once

#include "RenderPass.h"
#include "../../GLObject/ShaderStorageBuffer.h"

class RaytracePass : public RenderPass
{
public:
	RaytracePass(std::unordered_map<std::string, Texture*>& textures, const glm::ivec2& size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera) override;
	void restorePipelineImpl() override;

private:
	struct GLSLCamera
	{
		glm::vec3 position;
		float __padding0;
		glm::vec3 rayTL;
		float __padding1;
		glm::vec3 rayTR;
		float __padding2;
		glm::vec3 rayBL;
		float __padding3;
		glm::vec3 rayBR;
		float __padding4;
	};
	
	struct GLSLDirectionalLight
	{
		glm::vec3 fragToLightDirection;
		float angularDiameter;
		glm::vec3 color;
		float intensity;
	};
	
	struct GLSLPointLight
	{
		glm::vec3 position;
		float size;
		glm::vec3 color;
		float intensity;
	};
	
	struct GLSLSphere
	{
		glm::mat4 localToWorld;
		glm::mat4 worldToLocal;
		glm::mat4 localToWorldDirection;
		glm::mat4 worldToLocalDirection;
		glm::mat4 localToWorldNormal;
		glm::vec3 color;
		float __padding0;
	};
	
	ShaderProgram* _shader;
	Texture _rawRenderTexture;
	
	ShaderStorageBuffer<GLSLCamera> _cameraBuffer;
	ShaderStorageBuffer<GLSLDirectionalLight> _directionalLightBuffer;
	ShaderStorageBuffer<GLSLPointLight> _pointLightBuffer;
	ShaderStorageBuffer<GLSLSphere> _sphereBuffer;
};
