//#pragma once
//
//#include "Cyph3D/GLObject/Mesh.h"
//#include "Cyph3D/GLObject/GLMutableBuffer.h"
//#include "Cyph3D/GLObject/GLTexture.h"
//#include "Cyph3D/GLObject/GLShaderProgram.h"
//#include "Cyph3D/GLSL_types.h"
//#include "Cyph3D/Rendering/Pass/RenderPass.h"
//
//struct RenderRegistry;
//class Camera;
//
//struct RaytracePassInput
//{
//	RenderRegistry& registry;
//	Camera& camera;
//};
//
//struct RaytracePassOutput
//{
//	GLTexture& rawRender;
//	GLTexture& objectIndex;
//};
//
//class RaytracePass : public RenderPass<RaytracePassInput, RaytracePassOutput>
//{
//public:
//	explicit RaytracePass(const glm::uvec2& size);
//
//private:
//	struct GLSL_Material
//	{
//		GLSL_sampler2D albedo;
//		GLSL_sampler2D normal;
//		GLSL_sampler2D roughness;
//		GLSL_sampler2D metalness;
//		GLSL_sampler2D displacement;
//		GLSL_sampler2D emissive;
//	};
//
//	struct GLSL_Transform
//	{
//		GLSL_mat4 localToWorld;
//		GLSL_mat4 worldToLocal;
//		GLSL_mat4 localToWorldNormal;
//	};
//
//	struct GLSL_DirectionalLight
//	{
//		GLSL_vec3 fragToLightDirection;
//		GLSL_float angularDiameter;
//		GLSL_vec3 color;
//		GLSL_float intensity;
//		GLSL_bool castShadows;
//	};
//
//	struct GLSL_PointLight
//	{
//		GLSL_vec3 position;
//		GLSL_float radius;
//		GLSL_vec3 color;
//		GLSL_float intensity;
//		GLSL_bool castShadows;
//	};
//
//	struct GLSL_Sphere
//	{
//		GLSL_Material material;
//		GLSL_Transform transform;
//		GLSL_int objectIndex;
//		GLSL_bool contributeShadows;
//	};
//
//	struct GLSL_Plane
//	{
//		GLSL_Material material;
//		GLSL_Transform transform;
//		GLSL_bool infinite;
//		GLSL_int objectIndex;
//		GLSL_bool contributeShadows;
//	};
//
//	struct GLSL_Mesh
//	{
//		GLSL_Material material;
//		GLSL_Transform transform;
//		GLSL_int objectIndex;
//		GLSL_int vertexOffset;
//		GLSL_int indexOffset;
//		GLSL_int indexCount;
//		GLSL_bool contributeShadows;
//	};
//
//	GLShaderProgram _shader;
//	GLTexture _rawRenderTexture;
//	GLTexture _objectIndexTexture;
//
//	GLMutableBuffer<GLSL_DirectionalLight> _directionalLightBuffer;
//	GLMutableBuffer<GLSL_PointLight> _pointLightBuffer;
//	GLMutableBuffer<GLSL_Sphere> _sphereBuffer;
//	GLMutableBuffer<GLSL_Plane> _planeBuffer;
//	GLMutableBuffer<GLSL_Mesh> _meshBuffer;
//	GLMutableBuffer<Mesh::VertexData> _meshVertexDataBuffer;
//	GLMutableBuffer<GLuint> _meshIndexDataBuffer;
//
//	RaytracePassOutput renderImpl(RaytracePassInput& input) override;
//};