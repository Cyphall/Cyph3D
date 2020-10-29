#include <glad/glad.h>
#include "Renderer.h"
#include "../ResourceManagement/ResourceManager.h"
#include "../Window.h"
#include "../Scene/Camera.h"
#include "../Scene/Scene.h"
#include "../Engine.h"
#include "ToneMappingPostProcess.h"
#include "../GLStateManager.h"
#include "../Scene/MeshObject.h"
#include "../Helper/RenderHelper.h"
#include <glm/gtx/transform.hpp>

Renderer::Renderer():
// GBuffer
_gbuffer(Engine::getWindow().getSize()),
_normalTexture(TextureCreateInfo
{
	.size = _gbuffer.getSize(),
	.internalFormat = GL_RGB16F
}),
_colorTexture(TextureCreateInfo
{
	.size = _gbuffer.getSize(),
	.internalFormat = GL_RGB16F
}),
_materialTexture(TextureCreateInfo
{
	.size = _gbuffer.getSize(),
	.internalFormat = GL_RGBA8
}),
_geometryNormalTexture(TextureCreateInfo
{
	.size = _gbuffer.getSize(),
	.internalFormat = GL_RGB16F
}),
_depthTexture(TextureCreateInfo
{
	.size = _gbuffer.getSize(),
	.internalFormat = GL_DEPTH_COMPONENT24
}),

// Lighting pass
_resultFramebuffer(Engine::getWindow().getSize()),

// Post-processing
_resultTexture(TextureCreateInfo
{
	.size = _resultFramebuffer.getSize(),
	.internalFormat = GL_RGB16F
})
{
	// Main setup
	
	_gbuffer.attach(GL_COLOR_ATTACHMENT0, _normalTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT1, _colorTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT2, _materialTexture);
	_gbuffer.attach(GL_COLOR_ATTACHMENT3, _geometryNormalTexture);
	_gbuffer.attach(GL_DEPTH_ATTACHMENT, _depthTexture);
	
	ShaderProgramCreateInfo lightingShaderProgramCreateInfo;
	lightingShaderProgramCreateInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("lightingPass");
	lightingShaderProgramCreateInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("lightingPass");
	_lightingPassShader = Engine::getGlobalRM().requestShaderProgram(lightingShaderProgramCreateInfo);
	
	_resultFramebuffer.attach(GL_COLOR_ATTACHMENT0, _resultTexture);
	
	// Skybox pass setup
	
	std::vector<float> skyboxVBOData = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		
		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,
		
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	
	_skyboxVBO = std::make_unique<VertexBuffer<float>>(skyboxVBOData.size(), GL_DYNAMIC_STORAGE_BIT, 3);
	_skyboxVBO->setData(skyboxVBOData);
	
	_skyboxVAO.registerAttrib(*_skyboxVBO, 0, 3, GL_FLOAT, 0);
	
	ShaderProgramCreateInfo skyboxShaderProgramCreateInfo;
	skyboxShaderProgramCreateInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/skybox/skybox");
	skyboxShaderProgramCreateInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/skybox/skybox");
	_skyboxShader = Engine::getGlobalRM().requestShaderProgram(skyboxShaderProgramCreateInfo);
	
	// Post processing passes setup
	
	_postProcessEffects.emplace_back(std::make_unique<ToneMappingPostProcess>());
}

bool Renderer::getDebug() const
{
	return _debug;
}

void Renderer::setDebug(bool debug)
{
	_debug = debug;
}

void Renderer::render()
{
	Scene& scene = Engine::getScene();
	Camera& camera = scene.getCamera();
	
	std::vector<MeshObject*> meshObjects;
	std::vector<DirectionalLight*> directionalLights;
	std::vector<PointLight*> pointLights;
	for (std::unique_ptr<SceneObject>& object : Engine::getScene().getObjects())
	{
		MeshObject* meshObject = dynamic_cast<MeshObject*>(object.get());
		if (meshObject != nullptr)
		{
			meshObjects.push_back(meshObject);
		}
		
		DirectionalLight* directionalLight = dynamic_cast<DirectionalLight*>(object.get());
		if (directionalLight != nullptr)
		{
			directionalLights.push_back(directionalLight);
		}
		
		PointLight* pointLight = dynamic_cast<PointLight*>(object.get());
		if (pointLight != nullptr)
		{
			pointLights.push_back(pointLight);
		}
	}
	
	shadowMapPass(directionalLights, pointLights);
	
	updateLightBuffers(directionalLights, pointLights);
	
	_gbuffer.clearAll();
	
	firstPass(camera.getView(), camera.getProjection(), camera.getPosition(), meshObjects);
	if (scene.getSkybox() != nullptr && scene.getSkybox()->isResourceReady())
		skyboxPass(camera.getView(), camera.getProjection());
	lightingPass(camera.getPosition(), camera.getView(), camera.getProjection());
	
	Texture* finalRenderResult;
	if (_debug)
	{
		finalRenderResult = &_resultTexture;
	}
	else
	{
		finalRenderResult = postProcessingPass();
	}
	
	Framebuffer::drawToDefault(*finalRenderResult, true);
}

void Renderer::shadowMapPass(std::vector<DirectionalLight*> directionalLights, std::vector<PointLight*> pointLights)
{
	GLStateManager::push();
	
	GLStateManager::setDepthTest(true);
	GLStateManager::setDepthFunc(GL_LEQUAL);
	
	for (DirectionalLight* light : directionalLights)
	{
		light->updateShadowMap();
	}
	for (PointLight* light : pointLights)
	{
		light->updateShadowMap();
	}
	
	GLStateManager::pop();
}

void Renderer::updateLightBuffers(std::vector<DirectionalLight*> directionalLights, std::vector<PointLight*> pointLights)
{
	std::vector<DirectionalLight::LightData> directionalLightData;
	directionalLightData.reserve(directionalLights.size());
	for (DirectionalLight* light : directionalLights)
	{
		directionalLightData.push_back(light->getDataStruct());
	}
	_directionalLightsBuffer.setData(directionalLightData);
	
	std::vector<PointLight::LightData> pointLightData;
	pointLightData.reserve(pointLights.size());
	for (PointLight* light : pointLights)
	{
		pointLightData.push_back(light->getDataStruct());
	}
	_pointLightsBuffer.setData(pointLightData);
}

void Renderer::firstPass(glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos, std::vector<MeshObject*> meshObjects)
{
	GLStateManager::push();
	
	GLStateManager::setDepthTest(true);
	GLStateManager::setDepthFunc(GL_LEQUAL);
	
	GLStateManager::setCullFace(true);
	GLStateManager::setFrontFace(GL_CCW);
	
	_gbuffer.bind();
	
	for (MeshObject* meshObject : meshObjects)
	{
		meshObject->render(view, projection, viewPos);
	}
	
	GLStateManager::pop();
}

void Renderer::skyboxPass(glm::mat4 view, glm::mat4 projection)
{
	GLStateManager::push();
	
	GLStateManager::setDepthTest(true);
	GLStateManager::setDepthFunc(GL_LEQUAL);
	GLStateManager::setDepthMask(false);
	
	_gbuffer.bind();
	
	_skyboxShader->bind();
	
	glm::mat4 model = glm::rotate(glm::radians(Engine::getScene().getSkybox()->getRotation()), glm::vec3(0, 1, 0));
	_skyboxShader->setUniform("model", &model);
	glm::mat4 modifiedView = glm::mat4(glm::mat3(view));
	_skyboxShader->setUniform("view", &modifiedView);
	_skyboxShader->setUniform("projection", &projection);
	
	_skyboxShader->setUniform("skybox", &Engine::getScene().getSkybox()->getResource());
	
	_skyboxVAO.bind();
	
	glDrawArrays(GL_TRIANGLES, 0, 36);
	
	GLStateManager::pop();
}

void Renderer::lightingPass(glm::vec3 viewPos, glm::mat4 view, glm::mat4 projection)
{
	_pointLightsBuffer.bind(0);
	_directionalLightsBuffer.bind(1);
	
	_lightingPassShader->setUniform("viewPos", &viewPos);
	_lightingPassShader->setUniform("debug", &_debug);
	glm::mat4 viewProjectionInv = glm::inverse(projection * view);
	_lightingPassShader->setUniform("viewProjectionInv", &viewProjectionInv);
	
	_lightingPassShader->setUniform("normalTexture", &_normalTexture);
	_lightingPassShader->setUniform("colorTexture", &_colorTexture);
	_lightingPassShader->setUniform("materialTexture", &_materialTexture);
	_lightingPassShader->setUniform("geometryNormalTexture", &_geometryNormalTexture);
	_lightingPassShader->setUniform("depthTexture", &_depthTexture);
	
	_lightingPassShader->bind();
	_resultFramebuffer.bind();
	
	_resultFramebuffer.clearAll();
	
	RenderHelper::drawScreenQuad();
}

Texture* Renderer::postProcessingPass()
{
	Texture* renderResult = &_resultTexture;
	
	for (int i = 0; i < _postProcessEffects.size(); i++)
	{
		renderResult = _postProcessEffects[i]->render(renderResult, _resultTexture, _depthTexture);
	}
	
	return renderResult;
}
