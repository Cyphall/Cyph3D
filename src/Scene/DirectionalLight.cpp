#include "DirectionalLight.h"
#include "../GLStateManager.h"
#include "../Engine.h"
#include "../Scene/MeshObject.h"
#include "../Scene/Scene.h"

glm::mat4 DirectionalLight::_projection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, -50.0f, 50.0f);
GLPipelineState DirectionalLight::_pipelineState
{
	.depthTest = true,
	.colorMask = std::array<bool, 4>{false, false, false, false},
	.viewport = std::array<int, 4>{0, 0, _RESOLUTION, _RESOLUTION}
};

DirectionalLight::DirectionalLight(Transform* parent, const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 srgbColor, float intensity, bool castShadows):
		Light(parent, name, position, rotation, scale, srgbColor, intensity)
{
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/shadowMapping/directionalLight");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/shadowMapping/directionalLight");
	
	_shadowMapProgram = Engine::getScene().getRM().requestShaderProgram(createInfo);
	
	setCastShadows(castShadows);
}

DirectionalLight::DirectionalLight(Transform* parent, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec3 srgbColor, float intensity, bool castShadows):
		Light(parent, name, position, rotation, scale, srgbColor, intensity)
{
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/shadowMapping/directionalLight");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/shadowMapping/directionalLight");
	
	_shadowMapProgram = Engine::getScene().getRM().requestShaderProgram(createInfo);
	
	setCastShadows(castShadows);
}

void DirectionalLight::setCastShadows(bool value)
{
	if (_castShadows == value) return;
	
	_castShadows = value;
	
	if (value)
	{
		_shadowMapFb = std::make_unique<Framebuffer>(glm::ivec2(_RESOLUTION));
		
		TextureCreateInfo createInfo
		{
			.size = _shadowMapFb->getSize(),
			.internalFormat = GL_DEPTH_COMPONENT24,
			.isShadowMap = true
		};
		_shadowMap = std::make_unique<Texture>(createInfo);
		
		_shadowMapFb->attach(GL_DEPTH_ATTACHMENT, *_shadowMap.get());
	}
	else
	{
		_shadowMapFb.reset();
		_shadowMap.reset();
	}
}

bool DirectionalLight::getCastShadows() const
{
	return _castShadows;
}

DirectionalLight::LightData DirectionalLight::getDataStruct()
{
	return LightData
	{
		.fragToLightDirection = -getLightDirection(),
		.intensity = _intensity,
		.color = _linearColor,
		.castShadows = _castShadows,
		.lightViewProjection = _castShadows ? _viewProjection : glm::mat4(),
		.shadowMap = _castShadows ? _shadowMap->getBindlessHandle() : 0
	};
}

glm::vec3 DirectionalLight::getLightDirection()
{
	return glm::vec3(glm::mat4(glm::mat3(_transform.getWorldMatrix())) * glm::vec4(0, -1, 0, 1));
}

void DirectionalLight::updateShadowMap()
{
	if (!_castShadows) return;
	
	GLStateManager::use(_pipelineState);
	
	glm::vec3 worldPos = _transform.getWorldPosition();
	
	_viewProjection = _projection *
	                  glm::lookAt(
	                  		Engine::getScene().getCamera().position,
		                    Engine::getScene().getCamera().position + getLightDirection(),
	                  		glm::vec3(0, 1, 0));
	
	_shadowMapFb->bind();
	_shadowMapProgram->bind();
	_shadowMapProgram->setUniform("viewProjection", &_viewProjection);
	
	_shadowMapFb->clearDepth();
	
	for (auto& object : Engine::getScene().getObjects())
	{
		SceneObject* ptr = object.get();
		
		MeshObject* meshObject = dynamic_cast<MeshObject*>(ptr);
		if (meshObject != nullptr && meshObject->getContributeShadows())
		{
			if (meshObject->getModel() != nullptr && meshObject->getModel()->isResourceReady())
			{
				_shadowMapProgram->setUniform("model", &meshObject->getTransform().getWorldMatrix());
				meshObject->getModel()->render();
			}
		}
	}
}
