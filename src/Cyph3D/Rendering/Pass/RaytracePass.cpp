#include "RaytracePass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/GLObject/GLImmutableBuffer.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/Shape/MeshShape.h"
#include "Cyph3D/Rendering/Shape/PlaneShape.h"
#include "Cyph3D/Rendering/Shape/SphereShape.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/GLObject/GLCubemap.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Scene.h"

#include <glm/gtc/matrix_inverse.hpp>

RaytracePass::RaytracePass(const glm::uvec2& size):
RenderPass(size, "Raytrace pass"),
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
_meshIndexDataBuffer(GL_STREAM_DRAW),
_shader({
	{GL_COMPUTE_SHADER, "internal/raytracing/raytrace.comp"}
})
{

}

RaytracePassOutput RaytracePass::renderImpl(RaytracePassInput& input)
{

#pragma region Camera
	
	const std::array<glm::vec3, 4>& cornerRays = input.camera.getCornerRays();
	
	_shader.setUniform("u_camera.position", input.camera.getPosition());
	_shader.setUniform("u_camera.rayTL", cornerRays[0]);
	_shader.setUniform("u_camera.rayTR", cornerRays[1]);
	_shader.setUniform("u_camera.rayBL", cornerRays[2]);
	_shader.setUniform("u_camera.rayBR", cornerRays[3]);
	
#pragma endregion
#pragma region Skybox
	
	SkyboxAsset* skybox = Engine::getScene().getSkybox();
	
	if (skybox && skybox->isLoaded())
	{
		_shader.setUniform("u_skybox.enabled", true);
		_shader.setUniform("u_skybox.cubemap", skybox->getCubemap().getBindlessTextureHandle());
	}
	else
	{
		_shader.setUniform("u_skybox.enabled", false);
	}

#pragma endregion
#pragma region DirectionalLight
	
	std::vector<GLSL_DirectionalLight> glslDirectionalLights;
	for (const DirectionalLight::RenderData& renderData : input.registry.directionalLights)
	{
		GLSL_DirectionalLight& glslDirectionalLight = glslDirectionalLights.emplace_back();
		glslDirectionalLight.fragToLightDirection = renderData.fragToLightDirection;
		glslDirectionalLight.angularDiameter = renderData.angularDiameter;
		glslDirectionalLight.color = renderData.color;
		glslDirectionalLight.intensity = renderData.intensity;
		glslDirectionalLight.castShadows = renderData.castShadows;
	}
	_directionalLightBuffer.resize(glslDirectionalLights.size());
	_directionalLightBuffer.setData(glslDirectionalLights);
	_directionalLightBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	
#pragma endregion
#pragma region PointLight
	
	std::vector<GLSL_PointLight> glslPointLights;
	for (const PointLight::RenderData& renderData : input.registry.pointLights)
	{
		GLSL_PointLight& glslPointLight = glslPointLights.emplace_back();
		glslPointLight.position = renderData.pos;
		glslPointLight.radius = renderData.radius;
		glslPointLight.color = renderData.color;
		glslPointLight.intensity = renderData.intensity;
		glslPointLight.castShadows = renderData.castShadows;
	}
	_pointLightBuffer.resize(glslPointLights.size());
	_pointLightBuffer.setData(glslPointLights);
	_pointLightBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 1);
	
#pragma endregion
#pragma region Shape
	
	std::vector<GLSL_Sphere> glslSphereVec;
	std::vector<GLSL_Plane> glslPlaneVec;
	std::vector<GLSL_Mesh> glslMeshVec;
	std::vector<const MeshShape*> meshShapeVec;
	
	GLsizeiptr totalVertexCount = 0;
	GLsizeiptr totalIndexCount = 0;
	
	for (int i = 0; i < input.registry.shapes.size(); i++)
	{
		const ShapeRenderer::RenderData& renderData = input.registry.shapes[i];
		
		if (!renderData.shape->isReadyForRaytracingRender())
			continue;
		
		Transform& transform = renderData.owner->getTransform();

		MaterialAsset* material = renderData.material;
		if (material == nullptr)
		{
			material = MaterialAsset::getMissingMaterial();
		}
		else if (!material->isLoaded()) // should never happen, but we never know
		{
			material = MaterialAsset::getDefaultMaterial();
		}
		
		const SphereShape* sphereShape = dynamic_cast<const SphereShape*>(renderData.shape);
		if (sphereShape != nullptr)
		{
			GLSL_Sphere& glslSphere = glslSphereVec.emplace_back();
			glslSphere.material.albedo = material->getAlbedoTexture().getBindlessTextureHandle();
			glslSphere.material.normal = material->getNormalTexture().getBindlessTextureHandle();
			glslSphere.material.roughness = material->getRoughnessTexture().getBindlessTextureHandle();
			glslSphere.material.metalness = material->getMetalnessTexture().getBindlessTextureHandle();
			glslSphere.material.displacement = material->getDisplacementTexture().getBindlessTextureHandle();
			glslSphere.material.emissive = material->getEmissiveTexture().getBindlessTextureHandle();
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
			glslPlane.material.albedo = material->getAlbedoTexture().getBindlessTextureHandle();
			glslPlane.material.normal = material->getNormalTexture().getBindlessTextureHandle();
			glslPlane.material.roughness = material->getRoughnessTexture().getBindlessTextureHandle();
			glslPlane.material.metalness = material->getMetalnessTexture().getBindlessTextureHandle();
			glslPlane.material.displacement = material->getDisplacementTexture().getBindlessTextureHandle();
			glslPlane.material.emissive = material->getEmissiveTexture().getBindlessTextureHandle();
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
			glslMesh.material.albedo = material->getAlbedoTexture().getBindlessTextureHandle();
			glslMesh.material.normal = material->getNormalTexture().getBindlessTextureHandle();
			glslMesh.material.roughness = material->getRoughnessTexture().getBindlessTextureHandle();
			glslMesh.material.metalness = material->getMetalnessTexture().getBindlessTextureHandle();
			glslMesh.material.displacement = material->getDisplacementTexture().getBindlessTextureHandle();
			glslMesh.material.emissive = material->getEmissiveTexture().getBindlessTextureHandle();
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
	_sphereBuffer.resize(glslSphereVec.size());
	_sphereBuffer.setData(glslSphereVec);
	_sphereBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 2);
	_planeBuffer.resize(glslPlaneVec.size());
	_planeBuffer.setData(glslPlaneVec);
	_planeBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 3);
	
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
		
		const GLBuffer<Mesh::VertexData>& vbo = mesh.getVBO();
		GLsizeiptr vboElementCount = vbo.getCount();
		GLsizeiptr vboSize = vbo.getSize();
		
		glCopyNamedBufferSubData(vbo.getHandle(), _meshVertexDataBuffer.getHandle(), 0, currentVertexOffset * sizeof(Mesh::VertexData), vboSize);
		currentVertexOffset += vboElementCount;
		
		const GLBuffer<GLuint>& ibo = mesh.getIBO();
		GLsizeiptr iboElementCount = ibo.getCount();
		GLsizeiptr iboSize = ibo.getSize();
		
		glCopyNamedBufferSubData(ibo.getHandle(), _meshIndexDataBuffer.getHandle(), 0, currentIndexOffset * sizeof(GLuint), iboSize);
		currentIndexOffset += iboElementCount;
	}

	_meshBuffer.resize(glslMeshVec.size());
	_meshBuffer.setData(glslMeshVec);
	_meshBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 4);
	
	_meshVertexDataBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 5);
	_meshIndexDataBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 6);
	
#pragma endregion
	
	_shader.setUniform("u_resolution", _size);
	_shader.setUniform("o_renderImage", _rawRenderTexture.getBindlessImageHandle(GL_RGBA16F, GL_WRITE_ONLY));
	_shader.setUniform("o_objectIndexImage", _objectIndexTexture.getBindlessImageHandle(GL_R32I, GL_WRITE_ONLY));
	
	_shader.bind();
	_shader.dispatchAuto(glm::uvec3(_size, 1));
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	return RaytracePassOutput{
		.rawRender = _rawRenderTexture,
		.objectIndex = _objectIndexTexture
	};
}