#include "RaytracePass.h"
#include <glm/gtc/matrix_inverse.hpp>
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/Shape/SphereShape.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Rendering/Shape/PlaneShape.h"
#include "Cyph3D/Rendering/Shape/MeshShape.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/ShaderProgram.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/ResourceManagement/Skybox.h"
#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/GLObject/Material/Material.h"

RaytracePass::RaytracePass(std::unordered_map<std::string, Texture*>& textures, const glm::ivec2& size):
RenderPass(textures, size, "Raytrace pass"),
_rawRenderTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA16F
}),
_objectIndexTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_R32I
}),
_directionalLightBuffer(GL_STREAM_DRAW),
_pointLightBuffer(GL_STREAM_DRAW),
_sphereBuffer(GL_STREAM_DRAW),
_planeBuffer(GL_STREAM_DRAW),
_meshBuffer(GL_STREAM_DRAW),
_meshVertexDataBuffer(GL_STREAM_DRAW),
_meshIndexDataBuffer(GL_STREAM_DRAW)
{
	ShaderProgramCreateInfo lightingShaderProgramCreateInfo;
	lightingShaderProgramCreateInfo.shadersFiles[GL_COMPUTE_SHADER].emplace_back("internal/raytracing/raytrace");
	
	_shader = Engine::getGlobalRM().requestShaderProgram(lightingShaderProgramCreateInfo);
	
	textures["raw_render"] = &_rawRenderTexture;
	textures["objectIndex"] = &_objectIndexTexture;
}

void RaytracePass::preparePipelineImpl()
{

}

void RaytracePass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep)
{

#pragma region Camera
	
	const std::array<glm::vec3, 4>& cornerRays = camera.getCornerRays();
	
	_shader->setUniform("u_camera.position", camera.getPosition());
	_shader->setUniform("u_camera.rayTL", cornerRays[0]);
	_shader->setUniform("u_camera.rayTR", cornerRays[1]);
	_shader->setUniform("u_camera.rayBL", cornerRays[2]);
	_shader->setUniform("u_camera.rayBR", cornerRays[3]);
	
#pragma endregion
#pragma region Skybox
	
	Skybox* skybox = Engine::getScene().getSkybox();
	
	if (skybox && skybox->isResourceReady())
	{
		_shader->setUniform("u_skybox.enabled", true);
		_shader->setUniform("u_skybox.cubemap", skybox->getResource().getBindlessTextureHandle());
	}
	else
	{
		_shader->setUniform("u_skybox.enabled", false);
	}

#pragma endregion
#pragma region DirectionalLight
	
	std::vector<GLSL_DirectionalLight> glslDirectionalLights;
	for (const DirectionalLight::RenderData& renderData : objects.directionalLights)
	{
		GLSL_DirectionalLight& glslDirectionalLight = glslDirectionalLights.emplace_back();
		glslDirectionalLight.fragToLightDirection = renderData.fragToLightDirection;
		glslDirectionalLight.angularDiameter = renderData.angularDiameter;
		glslDirectionalLight.color = renderData.color;
		glslDirectionalLight.intensity = renderData.intensity;
		glslDirectionalLight.castShadows = renderData.castShadows;
	}
	_directionalLightBuffer.setData(glslDirectionalLights);
	_directionalLightBuffer.bind(0);
	
#pragma endregion
#pragma region PointLight
	
	std::vector<GLSL_PointLight> glslPointLights;
	for (const PointLight::RenderData& renderData : objects.pointLights)
	{
		GLSL_PointLight& glslPointLight = glslPointLights.emplace_back();
		glslPointLight.position = renderData.pos;
		glslPointLight.radius = renderData.radius;
		glslPointLight.color = renderData.color;
		glslPointLight.intensity = renderData.intensity;
		glslPointLight.castShadows = renderData.castShadows;
	}
	_pointLightBuffer.setData(glslPointLights);
	_pointLightBuffer.bind(1);
	
#pragma endregion
#pragma region Shape
	
	std::vector<GLSL_Sphere> glslSphereVec;
	std::vector<GLSL_Plane> glslPlaneVec;
	std::vector<GLSL_Mesh> glslMeshVec;
	std::vector<const MeshShape*> meshShapeVec;
	
	GLsizeiptr totalVertexCount = 0;
	GLsizeiptr totalIndexCount = 0;
	
	for (int i = 0; i < objects.shapes.size(); i++)
	{
		const ShapeRenderer::RenderData& renderData = objects.shapes[i];
		
		if (!renderData.shape->isReadyForRaytracingRender())
			continue;
		
		Transform& transform = renderData.owner->getTransform();
		
		const SphereShape* sphereShape = dynamic_cast<const SphereShape*>(renderData.shape);
		if (sphereShape != nullptr)
		{
			GLSL_Sphere& glslSphere = glslSphereVec.emplace_back();
			glslSphere.material.albedo = renderData.material->getTexture(MaterialMapType::ALBEDO).getBindlessTextureHandle();
			glslSphere.material.normal = renderData.material->getTexture(MaterialMapType::NORMAL).getBindlessTextureHandle();
			glslSphere.material.roughness = renderData.material->getTexture(MaterialMapType::ROUGHNESS).getBindlessTextureHandle();
			glslSphere.material.metalness = renderData.material->getTexture(MaterialMapType::METALNESS).getBindlessTextureHandle();
			glslSphere.material.displacement = renderData.material->getTexture(MaterialMapType::DISPLACEMENT).getBindlessTextureHandle();
			glslSphere.material.emissive = renderData.material->getTexture(MaterialMapType::EMISSIVE).getBindlessTextureHandle();
			glslSphere.transform.localToWorld = transform.getLocalToWorldMatrix();
			glslSphere.transform.worldToLocal = transform.getWorldToLocalMatrix();
			glslSphere.transform.localToWorldNormal = glm::inverseTranspose(glm::mat3(glslSphere.transform.localToWorld));
			glslSphere.objectIndex = i;
			glslSphere.contributeShadows = renderData.contributeShadows;
		}

		const PlaneShape* planeShape = dynamic_cast<const PlaneShape*>(renderData.shape);
		if (planeShape != nullptr)
		{
			GLSL_Plane& glslPlane = glslPlaneVec.emplace_back();
			glslPlane.material.albedo = renderData.material->getTexture(MaterialMapType::ALBEDO).getBindlessTextureHandle();
			glslPlane.material.normal = renderData.material->getTexture(MaterialMapType::NORMAL).getBindlessTextureHandle();
			glslPlane.material.roughness = renderData.material->getTexture(MaterialMapType::ROUGHNESS).getBindlessTextureHandle();
			glslPlane.material.metalness = renderData.material->getTexture(MaterialMapType::METALNESS).getBindlessTextureHandle();
			glslPlane.material.displacement = renderData.material->getTexture(MaterialMapType::DISPLACEMENT).getBindlessTextureHandle();
			glslPlane.material.emissive = renderData.material->getTexture(MaterialMapType::EMISSIVE).getBindlessTextureHandle();
			glslPlane.transform.localToWorld = transform.getLocalToWorldMatrix();
			glslPlane.transform.worldToLocal = transform.getWorldToLocalMatrix();
			glslPlane.transform.localToWorldNormal = glm::inverseTranspose(glm::mat3(glslPlane.transform.localToWorld));
			glslPlane.infinite = planeShape->isInfinite();
			glslPlane.objectIndex = i;
			glslPlane.contributeShadows = renderData.contributeShadows;
		}
		
		const MeshShape* meshShape = dynamic_cast<const MeshShape*>(renderData.shape);
		if (meshShape != nullptr)
		{
			GLSL_Mesh& glslMesh = glslMeshVec.emplace_back();
			glslMesh.material.albedo = renderData.material->getTexture(MaterialMapType::ALBEDO).getBindlessTextureHandle();
			glslMesh.material.normal = renderData.material->getTexture(MaterialMapType::NORMAL).getBindlessTextureHandle();
			glslMesh.material.roughness = renderData.material->getTexture(MaterialMapType::ROUGHNESS).getBindlessTextureHandle();
			glslMesh.material.metalness = renderData.material->getTexture(MaterialMapType::METALNESS).getBindlessTextureHandle();
			glslMesh.material.displacement = renderData.material->getTexture(MaterialMapType::DISPLACEMENT).getBindlessTextureHandle();
			glslMesh.material.emissive = renderData.material->getTexture(MaterialMapType::EMISSIVE).getBindlessTextureHandle();
			glslMesh.transform.localToWorld = transform.getLocalToWorldMatrix();
			glslMesh.transform.worldToLocal = transform.getWorldToLocalMatrix();
			glslMesh.transform.localToWorldNormal = glm::inverseTranspose(glm::mat3(glslMesh.transform.localToWorld));
			glslMesh.objectIndex = i;
			glslMesh.contributeShadows = renderData.contributeShadows;
			
			meshShapeVec.push_back(meshShape);
			
			const Mesh& mesh = meshShape->getMeshToRender();
			totalVertexCount += mesh.getVBO().getCount();
			glslMesh.indexCount = mesh.getIBO().getCount();
			totalIndexCount += glslMesh.indexCount;
			
		}
	}
	_sphereBuffer.setData(glslSphereVec);
	_sphereBuffer.bind(2);
	_planeBuffer.setData(glslPlaneVec);
	_planeBuffer.bind(3);
	
	_meshVertexDataBuffer.resize(totalVertexCount);
	_meshIndexDataBuffer.resize(totalIndexCount);
	
	int currentVertexOffset = 0;
	int currentIndexOffset = 0;
	
	for (int i = 0; i < meshShapeVec.size(); i++)
	{
		GLSL_Mesh& glslMesh = glslMeshVec[i];
		glslMesh.vertexOffset = currentVertexOffset;
		glslMesh.indexOffset = currentIndexOffset;
		
		const MeshShape* meshShape = meshShapeVec[i];
		
		const Mesh& mesh = meshShape->getMeshToRender();
		
		const Buffer<Mesh::VertexData>& vbo = mesh.getVBO();
		GLsizeiptr vboElementCount = vbo.getCount();
		GLsizeiptr vboSize = vbo.getSize();
		
		glCopyNamedBufferSubData(vbo.getHandle(), _meshVertexDataBuffer.getHandle(), 0, currentVertexOffset * sizeof(Mesh::VertexData), vboSize);
		currentVertexOffset += vboElementCount;
		
		const Buffer<GLuint>& ibo = mesh.getIBO();
		GLsizeiptr iboElementCount = ibo.getCount();
		GLsizeiptr iboSize = ibo.getSize();
		
		glCopyNamedBufferSubData(ibo.getHandle(), _meshIndexDataBuffer.getHandle(), 0, currentIndexOffset * sizeof(GLuint), iboSize);
		currentIndexOffset += iboElementCount;
	}
	
	_meshBuffer.setData(glslMeshVec);
	_meshBuffer.bind(4);
	
	_meshVertexDataBuffer.bind(5);
	_meshIndexDataBuffer.bind(6);
	
#pragma endregion
	
	_shader->setUniform("u_resolution", glm::uvec2(getSize()));
	_shader->setUniform("o_renderImage", _rawRenderTexture.getBindlessImageHandle(GL_RGBA16F, GL_WRITE_ONLY));
	_shader->setUniform("o_objectIndexImage", _objectIndexTexture.getBindlessImageHandle(GL_R32I, GL_WRITE_ONLY));
	
	_shader->bind();
	_shader->dispatchAuto(glm::ivec3(getSize(), 1));
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void RaytracePass::restorePipelineImpl()
{

}