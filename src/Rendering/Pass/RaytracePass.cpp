#include "RaytracePass.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"

RaytracePass::RaytracePass(std::unordered_map<std::string, Texture*>& textures, const glm::ivec2& size):
RenderPass(textures, size, "Raytrace pass"),
_rawRenderTexture(TextureCreateInfo
{
	.size = size,
	.internalFormat = GL_RGBA16F
})
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
	_shader->bind();
	_shader->setUniform("o_image", _rawRenderTexture.getBindlessImageHandle(GL_RGBA16F, GL_WRITE_ONLY));
	
	_shader->dispatch(glm::ivec3(getSize(), 1));
	
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void RaytracePass::restorePipelineImpl()
{

}
