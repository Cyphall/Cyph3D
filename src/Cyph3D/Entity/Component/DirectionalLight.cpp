#include "DirectionalLight.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Scene.h"

#include <glm/gtc/integer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

const char* const DirectionalLight::identifier = "DirectionalLight";
const vk::Format DirectionalLight::depthFormat = vk::Format::eD32Sfloat;

DirectionalLight::DirectionalLight(Entity& entity):
	LightBase(entity),
	_projection(glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, -50.0f, 50.0f))
{
	_projection[1][1] *= -1;
}

void DirectionalLight::setCastShadows(bool value)
{
	if (_castShadows == value) return;
	
	_castShadows = value;
	
	if (value)
	{
		_shadowMap = VKImage::createDynamic(
			Engine::getVKContext(),
			depthFormat,
			glm::uvec2(_resolution),
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
			vk::ImageAspectFlagBits::eDepth,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_shadowMapView = VKImageView::createDynamic(
			Engine::getVKContext(),
			_shadowMap,
			vk::ImageViewType::e2D);
	}
	else
	{
		_shadowMap = {};
		_shadowMapView = {};
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

void DirectionalLight::onPreRender(SceneRenderer& sceneRenderer, Camera& camera)
{
	RenderData data{};
	data.fragToLightDirection = -getLightDirection();
	data.intensity = getIntensity();
	data.color = getLinearColor();
	data.angularDiameter = getAngularDiameter();
	data.castShadows = _castShadows;
	if (_castShadows)
	{
		data.lightViewProjection = _projection *
								   glm::lookAt(
									   camera.getPosition(),
									   camera.getPosition() + getLightDirection(),
									   glm::vec3(0, 1, 0));
		data.shadowMapTexture = &_shadowMap;
		data.shadowMapTextureView = &_shadowMapView;
		data.shadowMapResolution = getResolution();
		data.shadowMapSize = _mapSize;
		data.shadowMapDepth = _mapDepth;
	}
	
	sceneRenderer.requestLightRendering(data);
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