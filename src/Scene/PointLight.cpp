#include "PointLight.h"
#include "../GLStateManager.h"
#include "../Engine.h"
#include "../Scene/MeshObject.h"
#include "../Scene/Scene.h"

glm::mat4 PointLight::_projection = glm::perspective(glm::radians(90.0f), 1.0f, _NEAR, _FAR);

PointLight::PointLight(Transform* parent, const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 srgbColor, float intensity, bool castShadows):
		Light(parent, name, position, rotation, scale, srgbColor, intensity)
{
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_GEOMETRY_SHADER].emplace_back("internal/shadowMapping/pointLight");
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/shadowMapping/pointLight");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/shadowMapping/pointLight");
	
	_shadowMapProgram = Engine::getScene().getRM().requestShaderProgram(createInfo);
	
	setCastShadows(castShadows);
}

PointLight::PointLight(Transform* parent, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec3 srgbColor, float intensity, bool castShadows):
		Light(parent, name, position, rotation, scale, srgbColor, intensity)
{
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_GEOMETRY_SHADER].emplace_back("internal/shadowMapping/pointLight");
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/shadowMapping/pointLight");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/shadowMapping/pointLight");
	
	_shadowMapProgram = Engine::getScene().getRM().requestShaderProgram(createInfo);
	
	setCastShadows(castShadows);
}

void PointLight::setCastShadows(bool value)
{
	if (_castShadows == value) return;
	
	_castShadows = value;
	
	if (value)
	{
		_shadowMapFb = std::make_unique<Framebuffer>(glm::ivec2(_RESOLUTION));
		
		CubemapCreateInfo createInfo
		{
			.size = _shadowMapFb->getSize(),
			.internalFormat = GL_DEPTH_COMPONENT24
		};
		_shadowMap = std::make_unique<Cubemap>(createInfo);
		
		_shadowMapFb->attach(GL_DEPTH_ATTACHMENT, *_shadowMap.get());
	}
	else
	{
		_shadowMapFb.reset();
		_shadowMap.reset();
	}
}

bool PointLight::getCastShadows() const
{
	return _castShadows;
}

PointLight::LightData PointLight::getDataStruct()
{
	return LightData
	{
		.pos = _transform.getWorldPosition(),
		.intensity = _intensity,
		.color = _linearColor,
		.castShadows = _castShadows,
		.shadowMap = _castShadows ? _shadowMap->getBindlessHandle() : 0,
		._far = _castShadows ? _FAR : 0
	};
}

void PointLight::updateShadowMap()
{
	if (!_castShadows) return;
	
	GLStateManager::push();
	GLStateManager::setViewport(0, 0, _RESOLUTION, _RESOLUTION);
	
	glm::vec3 worldPos = _transform.getWorldPosition();
	
	_viewProjections[0] = _projection *
						  glm::lookAt(worldPos, worldPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
	_viewProjections[1] = _projection *
	                      glm::lookAt(worldPos, worldPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
	_viewProjections[2] = _projection *
	                      glm::lookAt(worldPos, worldPos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
	_viewProjections[3] = _projection *
	                      glm::lookAt(worldPos, worldPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
	_viewProjections[4] = _projection *
	                      glm::lookAt(worldPos, worldPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
	_viewProjections[5] = _projection *
	                      glm::lookAt(worldPos, worldPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
	
	_shadowMapFb->bind();
	_shadowMapProgram->bind();
	_shadowMapProgram->setUniform("viewProjections", _viewProjections, 6);
	_shadowMapProgram->setUniform("lightPos", &worldPos);
	_shadowMapProgram->setUniform("far", &_FAR);
	
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
	
	GLStateManager::pop();
}
