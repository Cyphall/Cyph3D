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
	struct GLSLDirectionalLight
	{
		glm::vec3 fragToLightDirection;
		float angularDiameter;
		glm::vec3 color;
		float intensity;
		glm::vec3 __padding0;
		int32_t castShadows; // bool
	};
	
	struct GLSLPointLight
	{
		glm::vec3 position;
		float radius;
		glm::vec3 color;
		float intensity;
		glm::vec3 __padding0;
		int32_t castShadows; // bool
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
		glm::mat4 localToWorldNormal;
		int32_t objectIndex;
		int32_t contributeShadows;
		float padding0;
		float padding1;
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
		glm::mat4 localToWorldNormal;
		int32_t infinite;
		int32_t objectIndex;
		int32_t contributeShadows;
		float __padding0;
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
		glm::mat4 localToWorldNormal;
		int32_t objectIndex;
		int32_t vertexOffset;
		int32_t indexOffset;
		int32_t indexCount;
		int32_t contributeShadows;
		float __padding0;
		float __padding1;
		float __padding2;
	};
	
	ShaderProgram* _shader;
	Texture _rawRenderTexture;
	Texture _objectIndexTexture;
	
	ShaderStorageBuffer<GLSLDirectionalLight> _directionalLightBuffer;
	ShaderStorageBuffer<GLSLPointLight> _pointLightBuffer;
	ShaderStorageBuffer<GLSLSphere> _sphereBuffer;
	ShaderStorageBuffer<GLSLPlane> _planeBuffer;
	ShaderStorageBuffer<GLSLMeshInstanceData> _meshInstanceDataBuffer;
	ShaderStorageBuffer<Mesh::VertexData> _meshVertexDataBuffer;
	ShaderStorageBuffer<GLuint> _meshIndexDataBuffer;
};
