#include "LightingPass.h"
#include "../../Helper/RenderHelper.h"
#include "../../Rendering/Renderer.h"
#include "../../Window.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"

LightingPass::LightingPass(std::unordered_map<std::string, Texture*>& textures):
IRenderPass(textures),
_framebuffer(Engine::getWindow().getSize()),
_rawRenderTexture(TextureCreateInfo
 {
         .size = _framebuffer.getSize(),
         .internalFormat = GL_RGB16F
 })
{
	ShaderProgramCreateInfo lightingShaderProgramCreateInfo;
	lightingShaderProgramCreateInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("lightingPass");
	lightingShaderProgramCreateInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("lightingPass");
	
	_shader = Engine::getGlobalRM().requestShaderProgram(lightingShaderProgramCreateInfo);
	
	_framebuffer.attach(GL_COLOR_ATTACHMENT0, _rawRenderTexture);
	
	textures["raw_render"] = &_rawRenderTexture;
}

void LightingPass::preparePipeline()
{
	glEnable(GL_CULL_FACE);
}

void LightingPass::render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera)
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
	
	
	glm::vec3 pos = camera.getPosition();
	_shader->setUniform("viewPos", &pos);
	bool debug = Engine::getRenderer().getDebug();
	_shader->setUniform("debug", &debug);
	glm::mat4 viewProjectionInv = glm::inverse(camera.getProjection() * camera.getView());
	_shader->setUniform("viewProjectionInv", &viewProjectionInv);
	
	_shader->setUniform("normalTexture", textures["gbuffer_normal"]);
	_shader->setUniform("colorTexture", textures["gbuffer_color"]);
	_shader->setUniform("materialTexture", textures["gbuffer_material"]);
	_shader->setUniform("geometryNormalTexture", textures["gbuffer_gemoetryNormal"]);
	_shader->setUniform("depthTexture", textures["gbuffer_depth"]);
	
	_shader->bind();
	_framebuffer.bind();
	
	_framebuffer.clearAll();
	
	RenderHelper::drawScreenQuad();
}

void LightingPass::restorePipeline()
{
	glDisable(GL_CULL_FACE);
}
