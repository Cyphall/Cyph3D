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
		GLuint64 albedo;
		GLuint64 normal;
		GLuint64 roughness;
		GLuint64 metalness;
		GLuint64 displacement;
		GLuint64 emissive;
		glm::mat4 localToWorld;
		glm::mat4 worldToLocal;
		glm::mat4 localToWorldDirection;
		glm::mat4 worldToLocalDirection;
		glm::mat4 localToWorldNormal;
		int32_t objectIndex;
		float padding0;
		float padding1;
		float padding2;
	};
	
	struct GLSLPlane
	{
		GLuint64 albedo;
		GLuint64 normal;
		GLuint64 roughness;
		GLuint64 metalness;
		GLuint64 displacement;
		GLuint64 emissive;
		glm::mat4 localToWorld;
		glm::mat4 worldToLocal;
		glm::mat4 localToWorldDirection;
		glm::mat4 worldToLocalDirection;
		glm::mat4 localToWorldNormal;
		int32_t infinite;
		int32_t objectIndex;
		float __padding0;
		float __padding1;
	};
	
	struct GLSLMeshInstanceData
	{
		GLuint64 albedo;
		GLuint64 normal;
		GLuint64 roughness;
		GLuint64 metalness;
		GLuint64 displacement;
		GLuint64 emissive;
		glm::mat4 localToWorld;
		glm::mat4 worldToLocal;
		glm::mat4 localToWorldDirection;
		glm::mat4 worldToLocalDirection;
		glm::mat4 localToWorldNormal;
		int32_t objectIndex;
		int32_t vertexOffset;
		int32_t indexOffset;
		int32_t indexCount;
	};
	
	struct GLSLSkybox
	{
		int32_t enabled;
		GLuint64 cubemap;
	};
	
	ShaderProgram* _shader;
	Texture _rawRenderTexture;
	Texture _objectIndexTexture;
	
	ShaderStorageBuffer<GLSLCamera> _cameraBuffer;
	ShaderStorageBuffer<GLSLDirectionalLight> _directionalLightBuffer;
	ShaderStorageBuffer<GLSLPointLight> _pointLightBuffer;
	ShaderStorageBuffer<GLSLSphere> _sphereBuffer;
	ShaderStorageBuffer<GLSLPlane> _planeBuffer;
	ShaderStorageBuffer<GLSLMeshInstanceData> _meshInstanceDataBuffer;
	ShaderStorageBuffer<Mesh::VertexData> _meshVertexDataBuffer;
	ShaderStorageBuffer<GLuint> _meshIndexDataBuffer;
	ShaderStorageBuffer<GLSLSkybox> _skyboxBuffer;
};
