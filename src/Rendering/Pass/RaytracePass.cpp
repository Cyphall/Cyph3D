#include <glm/gtc/matrix_inverse.hpp>
#include "RaytracePass.h"
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
_cameraBuffer(1, GL_DYNAMIC_STORAGE_BIT)
{
	ShaderProgramCreateInfo lightingShaderProgramCreateInfo;
	lightingShaderProgramCreateInfo.shadersFiles[GL_COMPUTE_SHADER].emplace_back("internal/raytracing/raytrace");
	
	_shader = Engine::getGlobalRM().requestShaderProgram(lightingShaderProgramCreateInfo);
	
	textures["raw_render"] = &_rawRenderTexture;
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
	for (const ShapeRenderer::RenderData& renderData : objects.shapes)
	{
		Transform& transform = renderData.owner->getTransform();
		
		const SphereShape* sphereShape = dynamic_cast<const SphereShape*>(renderData.shape);
		if (sphereShape != nullptr)
		{
			GLSLSphere& glslSphere = glslSpheres.emplace_back();
			glslSphere.localToWorld = transform.getLocalToWorldMatrix();
			glslSphere.worldToLocal = transform.getWorldToLocalMatrix();
			glslSphere.localToWorldDirection = transform.getLocalToWorldDirectionMatrix();
			glslSphere.worldToLocalDirection = transform.getWorldToLocalDirectionMatrix();
			glslSphere.localToWorldNormal = glm::inverseTranspose(glslSphere.localToWorld);
			glslSphere.color = glm::vec3(1, 0, 0);
		}
		
		const PlaneShape* planeShape = dynamic_cast<const PlaneShape*>(renderData.shape);
		if (planeShape != nullptr)
		{
			GLSLPlane& glslPlane = glslPlanes.emplace_back();
			glslPlane.localToWorld = transform.getLocalToWorldMatrix();
			glslPlane.worldToLocal = transform.getWorldToLocalMatrix();
			glslPlane.localToWorldDirection = transform.getLocalToWorldDirectionMatrix();
			glslPlane.worldToLocalDirection = transform.getWorldToLocalDirectionMatrix();
			glslPlane.localToWorldNormal = glm::inverseTranspose(glslPlane.localToWorld);
			glslPlane.color = glm::vec3(0, 0, 1);
			glslPlane.infinite = planeShape->isInfinite();
		}
	}
	_sphereBuffer.setData(glslSpheres);
	_sphereBuffer.bind(3);
	_planeBuffer.setData(glslPlanes);
	_planeBuffer.bind(4);
	
	_shader->bind();
	_shader->setUniform("o_image", _rawRenderTexture.getBindlessImageHandle(GL_RGBA16F, GL_WRITE_ONLY));
	
	_shader->dispatch(glm::ivec3(getSize(), 1));
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void RaytracePass::restorePipelineImpl()
{

}
