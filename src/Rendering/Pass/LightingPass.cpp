#include "LightingPass.h"
#include "../../Helper/RenderHelper.h"
#include "../../Rendering/Renderer.h"
#include "../../Window.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"

LightingPass::LightingPass(std::unordered_map<std::string, Texture*>& textures):
RenderPass(textures, "Lighting pass"),
_framebuffer(Engine::getWindow().getSize()),
_rawRenderTexture(TextureCreateInfo
 {
         .size = _framebuffer.getSize(),
         .internalFormat = GL_RGB16F
 })
{
	ShaderProgramCreateInfo lightingShaderProgramCreateInfo;
	lightingShaderProgramCreateInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/lighting/lighting");
	lightingShaderProgramCreateInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/lighting/lighting");
	
	_shader = Engine::getGlobalRM().requestShaderProgram(lightingShaderProgramCreateInfo);
	
	_framebuffer.attachColor(_rawRenderTexture);
	_framebuffer.addToDrawBuffers(_rawRenderTexture, 0);
	
	textures["raw_render"] = &_rawRenderTexture;
}

void LightingPass::preparePipelineImpl()
{
	glEnable(GL_CULL_FACE);
}

void LightingPass::renderImpl(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
{
	std::vector<DirectionalLight::LightData> directionalLightData;
	directionalLightData.reserve(objects.directionalLights.size());
	for (DirectionalLight* light : objects.directionalLights)
	{
		directionalLightData.push_back(light->getDataStruct());
	}
	_directionalLightsBuffer.setData(directionalLightData);
	_directionalLightsBuffer.bind(1);
	
	std::vector<PointLight::LightData> pointLightData;
	pointLightData.reserve(objects.pointLights.size());
	for (PointLight* light : objects.pointLights)
	{
		pointLightData.push_back(light->getDataStruct());
	}
	_pointLightsBuffer.setData(pointLightData);
	_pointLightsBuffer.bind(0);
	
	
	_shader->setUniform("u_viewPos", camera.getPosition());
	_shader->setUniform("u_viewProjectionInv", glm::inverse(camera.getProjection() * camera.getView()));
	_shader->setUniform("u_time", (float)Engine::getTimer().time());
	
	_shader->setUniform("u_normalTexture", textures["gbuffer_normal"]);
	_shader->setUniform("u_colorTexture", textures["gbuffer_color"]);
	_shader->setUniform("u_materialTexture", textures["gbuffer_material"]);
	_shader->setUniform("u_geometryNormalTexture", textures["gbuffer_gemoetryNormal"]);
	_shader->setUniform("u_depthTexture", textures["z-prepass_depth"]);
	_shader->setUniform("u_position", textures["gbuffer_position"]);
	
	_shader->bind();
	_framebuffer.bindForDrawing();
	
	_rawRenderTexture.clear(GL_RGB, GL_HALF_FLOAT, nullptr);
	
	RenderHelper::drawScreenQuad();
}

void LightingPass::restorePipelineImpl()
{
	glDisable(GL_CULL_FACE);
}
