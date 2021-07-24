#include "RaytracePass.h"
#include <glm/gtc/matrix_inverse.hpp>
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../Shape/SphereShape.h"
#include "../../Entity/Entity.h"
#include "../Shape/PlaneShape.h"
#include "../Shape/MeshShape.h"

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
_meshInstanceDataBuffer(GL_STREAM_DRAW),
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

void RaytracePass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera)
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
	
	std::vector<GLSLDirectionalLight> glslDirectionalLights;
	for (const DirectionalLight::RenderData& renderData : objects.directionalLights)
	{
		GLSLDirectionalLight& glslDirectionalLight = glslDirectionalLights.emplace_back();
		glslDirectionalLight.fragToLightDirection = renderData.fragToLightDirection;
		glslDirectionalLight.angularDiameter = 1;
		glslDirectionalLight.color = renderData.color;
		glslDirectionalLight.intensity = renderData.intensity;
	}
	_directionalLightBuffer.setData(glslDirectionalLights);
	_directionalLightBuffer.bind(0);
	
#pragma endregion
#pragma region PointLight
	
	std::vector<GLSLPointLight> glslPointLights;
	for (const PointLight::RenderData& renderData : objects.pointLights)
	{
		GLSLPointLight& glslPointLight = glslPointLights.emplace_back();
		glslPointLight.position = renderData.pos;
		glslPointLight.size = 0.1;
		glslPointLight.color = renderData.color;
		glslPointLight.intensity = renderData.intensity;
	}
	_pointLightBuffer.setData(glslPointLights);
	_pointLightBuffer.bind(1);
	
#pragma endregion
#pragma region Shape
	
	std::vector<GLSLSphere> glslSphereVec;
	std::vector<GLSLPlane> glslPlaneVec;
	std::vector<GLSLMeshInstanceData> glslMeshInstanceDataVec;
	std::vector<const MeshShape*> meshShapeVec;
	
	int totalVertexCount = 0;
	int totalIndexCount = 0;
	
	for (int i = 0; i < objects.shapes.size(); i++)
	{
		const ShapeRenderer::RenderData& renderData = objects.shapes[i];
		
		if (!renderData.shape->isReadyForRaytracingRender())
			continue;
		
		Transform& transform = renderData.owner->getTransform();
		
		const SphereShape* sphereShape = dynamic_cast<const SphereShape*>(renderData.shape);
		if (sphereShape != nullptr)
		{
			GLSLSphere& glslSphere = glslSphereVec.emplace_back();
			glslSphere.albedo = renderData.material->getTexture(MaterialMapType::ALBEDO).getBindlessTextureHandle();
			glslSphere.normal = renderData.material->getTexture(MaterialMapType::NORMAL).getBindlessTextureHandle();
			glslSphere.roughness = renderData.material->getTexture(MaterialMapType::ROUGHNESS).getBindlessTextureHandle();
			glslSphere.metalness = renderData.material->getTexture(MaterialMapType::METALNESS).getBindlessTextureHandle();
			glslSphere.displacement = renderData.material->getTexture(MaterialMapType::DISPLACEMENT).getBindlessTextureHandle();
			glslSphere.emissive = renderData.material->getTexture(MaterialMapType::EMISSIVE).getBindlessTextureHandle();
			glslSphere.localToWorld = transform.getLocalToWorldMatrix();
			glslSphere.worldToLocal = transform.getWorldToLocalMatrix();
			glslSphere.localToWorldNormal = glm::inverseTranspose(glm::mat3(glslSphere.localToWorld));
			glslSphere.objectIndex = i;
		}

		const PlaneShape* planeShape = dynamic_cast<const PlaneShape*>(renderData.shape);
		if (planeShape != nullptr)
		{
			GLSLPlane& glslPlane = glslPlaneVec.emplace_back();
			glslPlane.albedo = renderData.material->getTexture(MaterialMapType::ALBEDO).getBindlessTextureHandle();
			glslPlane.normal = renderData.material->getTexture(MaterialMapType::NORMAL).getBindlessTextureHandle();
			glslPlane.roughness = renderData.material->getTexture(MaterialMapType::ROUGHNESS).getBindlessTextureHandle();
			glslPlane.metalness = renderData.material->getTexture(MaterialMapType::METALNESS).getBindlessTextureHandle();
			glslPlane.displacement = renderData.material->getTexture(MaterialMapType::DISPLACEMENT).getBindlessTextureHandle();
			glslPlane.emissive = renderData.material->getTexture(MaterialMapType::EMISSIVE).getBindlessTextureHandle();
			glslPlane.localToWorld = transform.getLocalToWorldMatrix();
			glslPlane.worldToLocal = transform.getWorldToLocalMatrix();
			glslPlane.localToWorldNormal = glm::inverseTranspose(glm::mat3(glslPlane.localToWorld));
			glslPlane.infinite = planeShape->isInfinite();
			glslPlane.objectIndex = i;
		}
		
		const MeshShape* meshShape = dynamic_cast<const MeshShape*>(renderData.shape);
		if (meshShape != nullptr)
		{
			GLSLMeshInstanceData& glslMeshInstanceData = glslMeshInstanceDataVec.emplace_back();
			glslMeshInstanceData.albedo = renderData.material->getTexture(MaterialMapType::ALBEDO).getBindlessTextureHandle();
			glslMeshInstanceData.normal = renderData.material->getTexture(MaterialMapType::NORMAL).getBindlessTextureHandle();
			glslMeshInstanceData.roughness = renderData.material->getTexture(MaterialMapType::ROUGHNESS).getBindlessTextureHandle();
			glslMeshInstanceData.metalness = renderData.material->getTexture(MaterialMapType::METALNESS).getBindlessTextureHandle();
			glslMeshInstanceData.displacement = renderData.material->getTexture(MaterialMapType::DISPLACEMENT).getBindlessTextureHandle();
			glslMeshInstanceData.emissive = renderData.material->getTexture(MaterialMapType::EMISSIVE).getBindlessTextureHandle();
			glslMeshInstanceData.localToWorld = transform.getLocalToWorldMatrix();
			glslMeshInstanceData.worldToLocal = transform.getWorldToLocalMatrix();
			glslMeshInstanceData.localToWorldNormal = glm::inverseTranspose(glm::mat3(glslMeshInstanceData.localToWorld));
			glslMeshInstanceData.objectIndex = i;
			
			meshShapeVec.push_back(meshShape);
			
			const Mesh& mesh = meshShape->getMeshToRender();
			totalVertexCount += mesh.getVBO().getCount();
			glslMeshInstanceData.indexCount = mesh.getIBO().getCount();
			totalIndexCount += glslMeshInstanceData.indexCount;
			
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
		GLSLMeshInstanceData& glslMeshInstanceData = glslMeshInstanceDataVec[i];
		glslMeshInstanceData.vertexOffset = currentVertexOffset;
		glslMeshInstanceData.indexOffset = currentIndexOffset;
		
		const MeshShape* meshShape = meshShapeVec[i];
		
		const Mesh& mesh = meshShape->getMeshToRender();
		
		const Buffer<Mesh::VertexData>& vbo = mesh.getVBO();
		int vboElementCount = vbo.getCount();
		int vboSize = vbo.getSize();
		
		glCopyNamedBufferSubData(vbo.getHandle(), _meshVertexDataBuffer.getHandle(), 0, currentVertexOffset * sizeof(Mesh::VertexData), vboSize);
		currentVertexOffset += vboElementCount;
		
		const Buffer<GLuint>& ibo = mesh.getIBO();
		int iboElementCount = ibo.getCount();
		int iboSize = ibo.getSize();
		
		glCopyNamedBufferSubData(ibo.getHandle(), _meshIndexDataBuffer.getHandle(), 0, currentIndexOffset * sizeof(GLuint), iboSize);
		currentIndexOffset += iboElementCount;
	}
	
	_meshInstanceDataBuffer.setData(glslMeshInstanceDataVec);
	_meshInstanceDataBuffer.bind(4);
	
	_meshVertexDataBuffer.bind(5);
	_meshIndexDataBuffer.bind(6);
	
#pragma endregion
	
	_shader->bind();
	_shader->setUniform("o_renderImage", _rawRenderTexture.getBindlessImageHandle(GL_RGBA16F, GL_WRITE_ONLY));
	_shader->setUniform("o_objectIndexImage", _objectIndexTexture.getBindlessImageHandle(GL_R32I, GL_WRITE_ONLY));
	
	_shader->dispatch(glm::ivec3(getSize(), 1));
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void RaytracePass::restorePipelineImpl()
{

}
