#include "RaytracePass.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"
#include "../Shape/SphereShape.h"
#include "../../Entity/Entity.h"

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
	
	std::vector<GLSLSphere> glslSpheres;
	for (const ShapeRenderer::RenderData& shapeData : objects.shapes)
	{
		const SphereShape* sphereShape = dynamic_cast<const SphereShape*>(shapeData.shape);
		if (sphereShape != nullptr)
		{
			Transform& transform = shapeData.owner->getTransform();
			
			GLSLSphere& glslSphere = glslSpheres.emplace_back();
			glslSphere.localToWorld = transform.getLocalToWorldMatrix();
			glslSphere.worldToLocal = transform.getWorldToLocalMatrix();
			glslSphere.localToWorldDirection = transform.getLocalToWorldDirectionMatrix();
			glslSphere.worldToLocalDirection = transform.getWorldToLocalDirectionMatrix();
			glslSphere.color = glm::vec3(1, 0, 0);
		}
	}
	
	_sphereBuffer.setData(glslSpheres);
	_sphereBuffer.bind(1);
	
	_shader->bind();
	_shader->setUniform("o_image", _rawRenderTexture.getBindlessImageHandle(GL_RGBA16F, GL_WRITE_ONLY));
	
	_shader->dispatch(glm::ivec3(getSize(), 1));
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void RaytracePass::restorePipelineImpl()
{

}
