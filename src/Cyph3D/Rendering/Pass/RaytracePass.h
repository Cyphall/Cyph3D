#pragma once

#include "Cyph3D/Rendering/Pass/RenderPass.h"
#include "Cyph3D/GLObject/ShaderStorageBuffer.h"
#include "Cyph3D/GLSL_types.h"
#include "Cyph3D/GLObject/Texture.h"
#include "Cyph3D/GLObject/Mesh.h"

class ShaderProgram;

class RaytracePass : public RenderPass
{
public:
	RaytracePass(std::unordered_map<std::string, Texture*>& textures, const glm::ivec2& size);
	
	void preparePipelineImpl() override;
	void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep) override;
	void restorePipelineImpl() override;

private:
	struct GLSL_Material
	{
		GLSL_sampler2D albedo;
		GLSL_sampler2D normal;
		GLSL_sampler2D roughness;
		GLSL_sampler2D metalness;
		GLSL_sampler2D displacement;
		GLSL_sampler2D emissive;
	};
	
	struct GLSL_Transform
	{
		GLSL_mat4 localToWorld;
		GLSL_mat4 worldToLocal;
		GLSL_mat4 localToWorldNormal;
	};
	
	struct GLSL_DirectionalLight
	{
		GLSL_vec3 fragToLightDirection;
		GLSL_float angularDiameter;
		GLSL_vec3 color;
		GLSL_float intensity;
		GLSL_bool castShadows;
	};
	
	struct GLSL_PointLight
	{
		GLSL_vec3 position;
		GLSL_float radius;
		GLSL_vec3 color;
		GLSL_float intensity;
		GLSL_bool castShadows;
	};
	
	struct GLSL_Sphere
	{
		GLSL_Material material;
		GLSL_Transform transform;
		GLSL_int objectIndex;
		GLSL_bool contributeShadows;
	};
	
	struct GLSL_Plane
	{
		GLSL_Material material;
		GLSL_Transform transform;
		GLSL_bool infinite;
		GLSL_int objectIndex;
		GLSL_bool contributeShadows;
	};
	
	struct GLSL_Mesh
	{
		GLSL_Material material;
		GLSL_Transform transform;
		GLSL_int objectIndex;
		GLSL_int vertexOffset;
		GLSL_int indexOffset;
		GLSL_int indexCount;
		GLSL_bool contributeShadows;
	};
	
	ShaderProgram* _shader;
	Texture _rawRenderTexture;
	Texture _objectIndexTexture;
	
	ShaderStorageBuffer<GLSL_DirectionalLight> _directionalLightBuffer;
	ShaderStorageBuffer<GLSL_PointLight> _pointLightBuffer;
	ShaderStorageBuffer<GLSL_Sphere> _sphereBuffer;
	ShaderStorageBuffer<GLSL_Plane> _planeBuffer;
	ShaderStorageBuffer<GLSL_Mesh> _meshBuffer;
	ShaderStorageBuffer<Mesh::VertexData> _meshVertexDataBuffer;
	ShaderStorageBuffer<GLuint> _meshIndexDataBuffer;
};