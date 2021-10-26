#include "LightingPass.h"
#include "../../Helper/RenderHelper.h"
#include "../../Rendering/Renderer/Renderer.h"
#include "../../Window.h"
#include "../../ResourceManagement/ResourceManager.h"
#include "../../Engine.h"

LightingPass::LightingPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size):
RenderPass(textures, size, "Lighting pass"),
_framebuffer(size),
_rawRenderTexture(TextureCreateInfo
{
 .size = size,
 .internalFormat = GL_RGBA16F
})
{
	ShaderProgramCreateInfo lightingShaderProgramCreateInfo;
	lightingShaderProgramCreateInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/fullscreen quad");
	lightingShaderProgramCreateInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/lighting/lighting");
	
	_shader = Engine::getGlobalRM().requestShaderProgram(lightingShaderProgramCreateInfo);
	
	_framebuffer.attachColor(0, _rawRenderTexture);
	_framebuffer.addToDrawBuffers(0, 0);
	
	textures["raw_render"] = &_rawRenderTexture;
}

void LightingPass::preparePipelineImpl()
{
	glEnable(GL_CULL_FACE);
}

void LightingPass::renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& registry, Camera& camera, PerfStep& previousFramePerfStep)
{
	std::vector<GLSLDirectionalLight> directionalLightData;
	directionalLightData.reserve(registry.directionalLights.size());
	for (DirectionalLight::RenderData& renderData : registry.directionalLights)
	{
		GLSLDirectionalLight data;
		data.fragToLightDirection = renderData.fragToLightDirection;
		data.intensity = renderData.intensity;
		data.color = renderData.color;
		data.castShadows = renderData.castShadows;
		if (renderData.castShadows)
		{
			data.lightViewProjection = renderData.lightViewProjection;
			data.shadowMap = renderData.shadowMapTexture->getBindlessTextureHandle();
			data.mapSize = renderData.mapSize;
			data.mapDepth = renderData.mapDepth;
		}
		directionalLightData.push_back(data);
	}
	_directionalLightsBuffer.setData(directionalLightData);
	_directionalLightsBuffer.bind(1);
	
	std::vector<GLSLPointLight> pointLightData;
	pointLightData.reserve(registry.pointLights.size());
	for (PointLight::RenderData& renderData : registry.pointLights)
	{
		GLSLPointLight data;
		data.pos = renderData.pos;
		data.intensity = renderData.intensity;
		data.color = renderData.color;
		data.castShadows = renderData.castShadows;
		if (renderData.castShadows)
		{
			data.shadowMap = renderData.shadowMapTexture->getBindlessTextureHandle();
			data.far = renderData.far;
			data.maxTexelSizeAtUnitDistance = 2.0f / renderData.mapResolution;
		}
		pointLightData.push_back(data);
	}
	_pointLightsBuffer.setData(pointLightData);
	_pointLightsBuffer.bind(0);
	
	
	_shader->setUniform("u_viewPos", camera.getPosition());
	_shader->setUniform("u_viewProjectionInv", glm::inverse(camera.getProjection() * camera.getView()));
	_shader->setUniform("u_time", (float)Engine::getTimer().time());
	
	_shader->setUniform("u_normalTexture", textures["gbuffer_normal"]->getBindlessTextureHandle());
	_shader->setUniform("u_colorTexture", textures["gbuffer_color"]->getBindlessTextureHandle());
	_shader->setUniform("u_materialTexture", textures["gbuffer_material"]->getBindlessTextureHandle());
	_shader->setUniform("u_geometryNormalTexture", textures["gbuffer_gemoetryNormal"]->getBindlessTextureHandle());
	_shader->setUniform("u_depthTexture", textures["z-prepass_depth"]->getBindlessTextureHandle());
	_shader->setUniform("u_position", textures["gbuffer_position"]->getBindlessTextureHandle());
	
	_shader->bind();
	_framebuffer.bindForDrawing();
	
	_rawRenderTexture.clear(GL_RGBA, GL_HALF_FLOAT, nullptr);
	
	RenderHelper::drawScreenQuad();
}

void LightingPass::restorePipelineImpl()
{
	glDisable(GL_CULL_FACE);
}
