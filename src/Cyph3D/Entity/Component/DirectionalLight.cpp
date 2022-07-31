#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/Entity/Entity.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>
#include <imgui.h>
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/Renderer/Renderer.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/Framebuffer.h"
#include "Cyph3D/GLObject/Texture.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/RenderContext.h"
#include "Cyph3D/Scene/Camera.h"

const char* DirectionalLight::identifier = "DirectionalLight";
glm::mat4 DirectionalLight::_projection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, -50.0f, 50.0f);

DirectionalLight::DirectionalLight(Entity& entity):
LightBase(entity)
{

}

void DirectionalLight::setCastShadows(bool value)
{
	if (_castShadows == value) return;
	
	_castShadows = value;
	
	if (value)
	{
		_shadowMapFb = std::make_unique<Framebuffer>(glm::ivec2(_resolution));
		
		TextureCreateInfo textureCreateInfo;
		textureCreateInfo.size = _shadowMapFb->getSize();
		textureCreateInfo.internalFormat = GL_DEPTH_COMPONENT32;
		textureCreateInfo.minFilter = GL_NEAREST;
		textureCreateInfo.magFilter = GL_NEAREST;
		textureCreateInfo.wrapS = GL_CLAMP_TO_BORDER;
		textureCreateInfo.wrapT = GL_CLAMP_TO_BORDER;
		textureCreateInfo.borderColor = {1, 1, 1, 1};
		_shadowMap = std::make_unique<Texture>(textureCreateInfo);
		
		_shadowMapFb->attachDepth(*_shadowMap.get());
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

glm::vec3 DirectionalLight::getLightDirection()
{
	return -getTransform().getUp();
}

void DirectionalLight::setResolution(int value)
{
	bool castShadows = getCastShadows();
	
	if (castShadows)
		setCastShadows(false);
	
	_resolution = value;
	
	if (castShadows)
		setCastShadows(true);
}

int DirectionalLight::getResolution() const
{
	return _resolution;
}

ObjectSerialization DirectionalLight::serialize() const
{
	ObjectSerialization serialization;
	serialization.version = 2;
	serialization.identifier = getIdentifier();
	
	glm::vec3 color = getSrgbColor();
	serialization.data["color"] = {color.r, color.g, color.b};
	serialization.data["intensity"] = getIntensity();
	serialization.data["cast_shadows"] = getCastShadows();
	serialization.data["shadow_resolution"] = getResolution();
	serialization.data["angular_diameter"] = getAngularDiameter();
	
	return serialization;
}

void DirectionalLight::deserialize(const ObjectSerialization& serialization)
{
	setSrgbColor(glm::make_vec3(serialization.data["color"].get<std::vector<float>>().data()));
	setIntensity(serialization.data["intensity"].get<float>());
	setCastShadows(serialization.data["cast_shadows"].get<bool>());
	setResolution(serialization.data["shadow_resolution"].get<int>());
	
	if (serialization.version >= 2)
	{
		setAngularDiameter(serialization.data["angular_diameter"].get<float>());
	}
}

void DirectionalLight::onPreRender(RenderContext& context)
{
	RenderData data;
	data.fragToLightDirection = -getLightDirection();
	data.intensity = getIntensity();
	data.color = getLinearColor();
	data.angularDiameter = getAngularDiameter();
	data.castShadows = _castShadows;
	if (_castShadows)
	{
		data.lightViewProjection = _projection *
								   glm::lookAt(
									   context.camera.getPosition(),
									   context.camera.getPosition() + getLightDirection(),
									   glm::vec3(0, 1, 0));
		data.shadowMapTexture = _shadowMap.get();
		data.shadowMapFramebuffer = _shadowMapFb.get();
		data.mapResolution = getResolution();
		data.mapSize = _mapSize;
		data.mapDepth = _mapDepth;
	}
	
	context.renderer.requestLightRendering(data);
}

void DirectionalLight::onDrawUi()
{
	glm::vec3 imGuiSrgbColor = getSrgbColor();
	if (ImGui::ColorEdit3("Color", glm::value_ptr(imGuiSrgbColor)))
	{
		setSrgbColor(imGuiSrgbColor);
	}
	
	float intensity = getIntensity();
	if (ImGui::DragFloat("Intensity", &intensity, 0.01f))
	{
		setIntensity(intensity);
	}
	
	float angularDiameter = getAngularDiameter();
	if (ImGui::SliderFloat("Angular Diameter", &angularDiameter, 0.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
	{
		setAngularDiameter(angularDiameter);
	}
	
	bool castShadows = getCastShadows();
	if (ImGui::Checkbox("Cast Shadows", &castShadows))
	{
		setCastShadows(castShadows);
	}
	
	std::array<const char*, 8> resolutions = {"64", "128", "256", "512", "1024", "2048", "4096", "8192"};
	int currIndex = glm::log2(getResolution() / 64);
	if (ImGui::BeginCombo("Shadow Resolution", resolutions[currIndex]))
	{
		for (int i = 0; i < resolutions.size(); i++)
		{
			bool selected = i == currIndex;
			if (ImGui::Selectable(resolutions[i], selected))
			{
				currIndex = i;
				setResolution(64 * glm::pow(2, i));
			}
			
			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		
		ImGui::EndCombo();
	}
}

const char* DirectionalLight::getIdentifier() const
{
	return identifier;
}

void DirectionalLight::duplicate(Entity& targetEntity) const
{
	DirectionalLight& newComponent = targetEntity.addComponent<DirectionalLight>();
	newComponent.setSrgbColor(getSrgbColor());
	newComponent.setIntensity(getIntensity());
	newComponent.setCastShadows(getCastShadows());
	newComponent.setResolution(getResolution());
	newComponent.setAngularDiameter(getAngularDiameter());
}

float DirectionalLight::getAngularDiameter() const
{
	return _angularDiameter;
}

void DirectionalLight::setAngularDiameter(float value)
{
	_angularDiameter = value;
}