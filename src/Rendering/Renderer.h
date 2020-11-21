#pragma once

#include "../GLObject/Framebuffer.h"
#include "../GLObject/VertexArray.h"
#include "../GLObject/VertexBuffer.h"
#include "../GLObject/ShaderStorageBuffer.h"
#include "../Scene/PointLight.h"
#include "../Scene/DirectionalLight.h"
#include "../Scene/MeshObject.h"
#include "IPostProcessEffect.h"

class Renderer
{
public:
	Renderer();
	
	bool getDebug() const;
	void setDebug(bool debug);
	
	void render();

private:
	// GBuffer
	Framebuffer _gbuffer;
	Texture _normalTexture;
	Texture _colorTexture;
	Texture _materialTexture;
	Texture _geometryNormalTexture;
	Texture _depthTexture;
	
	GLPipelineState _firstPassPipelineState;
	bool _debug = false;
	
	// Skybox
	VertexArray _skyboxVAO;
	std::unique_ptr<VertexBuffer<float>> _skyboxVBO;
	ShaderProgram* _skyboxShader;
	GLPipelineState _skyboxPassPipelineState;
	
	// Lighting pass
	ShaderProgram* _lightingPassShader;
	ShaderStorageBuffer<PointLight::LightData> _pointLightsBuffer;
	ShaderStorageBuffer<DirectionalLight::LightData> _directionalLightsBuffer;
	Framebuffer _resultFramebuffer;
	
	// Post-processing
	Texture _resultTexture;
	std::vector<std::unique_ptr<IPostProcessEffect>> _postProcessEffects;
	
	void shadowMapPass(std::vector<DirectionalLight*> directionalLights, std::vector<PointLight*> pointLights);
	void updateLightBuffers(std::vector<DirectionalLight*> directionalLights, std::vector<PointLight*> pointLights);
	void firstPass(glm::mat4 view, glm::mat4 projection, glm::vec3 viewPos, std::vector<MeshObject*> meshObjects);
	void skyboxPass(glm::mat4 view, glm::mat4 projection);
	void lightingPass(glm::vec3 viewPos, glm::mat4 view, glm::mat4 projection);
	Texture* postProcessingPass();
};
