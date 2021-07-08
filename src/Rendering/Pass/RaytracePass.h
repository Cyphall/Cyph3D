#pragma once

#include "RenderPass.h"
#include "../../GLObject/ShaderStorageBuffer.h"

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

struct GLSLSphere
{
	glm::mat4 localToWorld;
	glm::mat4 worldToLocal;
	glm::mat4 localToWorldRotation;
	glm::mat4 worldToLocalRotation;
	glm::vec3 color;
	float __padding0;
};

class RaytracePass : public RenderPass
{
public:
	RaytracePass(std::unordered_map<std::string, Texture*>& textures, const glm::ivec2& size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera) override;
	void restorePipelineImpl() override;

private:
	
	ShaderProgram* _shader;
	Texture _rawRenderTexture;
	
	ShaderStorageBuffer<GLSLCamera> _cameraBuffer;
	ShaderStorageBuffer<GLSLSphere> _sphereBuffer;
};
