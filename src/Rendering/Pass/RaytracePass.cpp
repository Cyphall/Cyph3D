#include "RaytracePass.h"
#include <glm/gtc/matrix_inverse.hpp>
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"
#include "../Shape/SphereShape.h"
#include "../../Entity/Entity.h"
#include "../Shape/PlaneShape.h"

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
_cameraBuffer(1, GL_DYNAMIC_STORAGE_BIT)
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
	const std::array<glm::vec3, 4>& cornerRays = camera.getCornerRays();
	
	GLSLCamera glslCamera;
	glslCamera.position = camera.getPosition();
	glslCamera.rayTL = cornerRays[0];
	glslCamera.rayTR = cornerRays[1];
	glslCamera.rayBL = cornerRays[2];
	glslCamera.rayBR = cornerRays[3];
	
	_cameraBuffer.setData(&glslCamera, 1);
	_cameraBuffer.bind(0);
	
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
	_directionalLightBuffer.bind(1);
	
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
	_pointLightBuffer.bind(2);
	
	std::vector<GLSLSphere> glslSpheres;
	std::vector<GLSLPlane> glslPlanes;
	for (int i = 0; i < objects.shapes.size(); i++)
	{
		const ShapeRenderer::RenderData& renderData = objects.shapes[i];
		
		Transform& transform = renderData.owner->getTransform();
		
		const SphereShape* sphereShape = dynamic_cast<const SphereShape*>(renderData.shape);
		if (sphereShape != nullptr)
		{
			GLSLSphere& glslSphere = glslSpheres.emplace_back();
			glslSphere.albedo = renderData.material->getTexture(MaterialMapType::ALBEDO).getBindlessTextureHandle();
			glslSphere.normal = renderData.material->getTexture(MaterialMapType::NORMAL).getBindlessTextureHandle();
			glslSphere.roughness = renderData.material->getTexture(MaterialMapType::ROUGHNESS).getBindlessTextureHandle();
			glslSphere.metalness = renderData.material->getTexture(MaterialMapType::METALNESS).getBindlessTextureHandle();
			glslSphere.displacement = renderData.material->getTexture(MaterialMapType::DISPLACEMENT).getBindlessTextureHandle();
			glslSphere.emissive = renderData.material->getTexture(MaterialMapType::EMISSIVE).getBindlessTextureHandle();
			glslSphere.localToWorld = transform.getLocalToWorldMatrix();
			glslSphere.worldToLocal = transform.getWorldToLocalMatrix();
			glslSphere.localToWorldDirection = transform.getLocalToWorldDirectionMatrix();
			glslSphere.worldToLocalDirection = transform.getWorldToLocalDirectionMatrix();
			glslSphere.localToWorldNormal = glm::inverseTranspose(glslSphere.localToWorld);
			glslSphere.objectIndex = i;
		}
		
		const PlaneShape* planeShape = dynamic_cast<const PlaneShape*>(renderData.shape);
		if (planeShape != nullptr)
		{
			GLSLPlane& glslPlane = glslPlanes.emplace_back();
			glslPlane.albedo = renderData.material->getTexture(MaterialMapType::ALBEDO).getBindlessTextureHandle();
			glslPlane.normal = renderData.material->getTexture(MaterialMapType::NORMAL).getBindlessTextureHandle();
			glslPlane.roughness = renderData.material->getTexture(MaterialMapType::ROUGHNESS).getBindlessTextureHandle();
			glslPlane.metalness = renderData.material->getTexture(MaterialMapType::METALNESS).getBindlessTextureHandle();
			glslPlane.displacement = renderData.material->getTexture(MaterialMapType::DISPLACEMENT).getBindlessTextureHandle();
			glslPlane.emissive = renderData.material->getTexture(MaterialMapType::EMISSIVE).getBindlessTextureHandle();
			glslPlane.localToWorld = transform.getLocalToWorldMatrix();
			glslPlane.worldToLocal = transform.getWorldToLocalMatrix();
			glslPlane.localToWorldDirection = transform.getLocalToWorldDirectionMatrix();
			glslPlane.worldToLocalDirection = transform.getWorldToLocalDirectionMatrix();
			glslPlane.localToWorldNormal = glm::inverseTranspose(glslPlane.localToWorld);
			glslPlane.infinite = planeShape->isInfinite();
			glslPlane.objectIndex = i;
		}
	}
	_sphereBuffer.setData(glslSpheres);
	_sphereBuffer.bind(3);
	_planeBuffer.setData(glslPlanes);
	_planeBuffer.bind(4);
	
	_shader->bind();
	_shader->setUniform("o_renderImage", _rawRenderTexture.getBindlessImageHandle(GL_RGBA16F, GL_WRITE_ONLY));
	_shader->setUniform("o_objectIndexImage", _objectIndexTexture.getBindlessImageHandle(GL_R32I, GL_WRITE_ONLY));
	
	_shader->dispatch(glm::ivec3(getSize(), 1));
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void RaytracePass::restorePipelineImpl()
{

}
