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

static const float SHADOW_MAP_WORLD_SIZE = 60.0f;
static const float SHADOW_MAP_WORLD_DEPTH = 100.0f;

DirectionalLight::DirectionalLight(Entity& entity):
	LightBase(entity),
	_projection(glm::ortho(-SHADOW_MAP_WORLD_SIZE / 2, SHADOW_MAP_WORLD_SIZE / 2, -SHADOW_MAP_WORLD_SIZE / 2, SHADOW_MAP_WORLD_SIZE / 2, -SHADOW_MAP_WORLD_DEPTH / 2, SHADOW_MAP_WORLD_DEPTH / 2))
{
	_projection[1][1] *= -1;
}

void DirectionalLight::setCastShadows(bool value)
{
	if (_castShadows == value) return;
	
	_castShadows = value;
	
	if (value)
	{
		VKImageInfo imageInfo(
			depthFormat,
			glm::uvec2(_resolution),
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_shadowMap = VKImage::create(Engine::getVKContext(), imageInfo);
		
		VKImageViewInfo imageViewInfo(
			_shadowMap,
			vk::ImageViewType::e2D);
		
		_shadowMapView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
	}
	else
	{
		_shadowMap = {};
		_shadowMapView = {};
	}
	
	_changed();
}

bool DirectionalLight::getCastShadows() const
{
	return _castShadows;
}

void DirectionalLight::setResolution(int value)
{
	bool castShadows = getCastShadows();
	
	if (castShadows)
		setCastShadows(false);
	
	_resolution = value;
	
	if (castShadows)
		setCastShadows(true);
	
	_changed();
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
	switch (serialization.version)
	{
		case 1:
			deserializeFromVersion1(serialization.data);
			break;
		case 2:
			deserializeFromVersion2(serialization.data);
			break;
		default:
			throw;
	}
}

void DirectionalLight::onPreRender(RenderRegistry& renderRegistry, Camera& camera)
{
	RenderData data{};
	data.fragToLightDirection = getTransform().getUp();
	data.intensity = getIntensity();
	data.color = getLinearColor();
	data.angularDiameter = getAngularDiameter();
	data.castShadows = _castShadows;
	if (_castShadows)
	{
		float texelWorldSize = SHADOW_MAP_WORLD_SIZE / _resolution;
		glm::mat4 worldToShadowMapTexel = glm::ortho<float>(-texelWorldSize, texelWorldSize, -texelWorldSize, texelWorldSize, 0, 1) *
		                                  glm::lookAt(glm::vec3(0, 0, 0), getTransform().getDown(), getTransform().getForward());
		
		glm::vec4 shadowMapTexelPos4D = worldToShadowMapTexel * glm::vec4(camera.getPosition(), 1);
		glm::vec3 shadowMapTexelPos = glm::vec3(shadowMapTexelPos4D) / shadowMapTexelPos4D.w;
		
		glm::vec3 roundedShadowMapTexelPos = glm::vec3(glm::round(glm::vec2(shadowMapTexelPos)), shadowMapTexelPos.z);
		
		glm::vec4 shadowMapRoundedWorldPos4D = glm::inverse(worldToShadowMapTexel) * glm::vec4(roundedShadowMapTexelPos, 1);
		glm::vec3 shadowMapRoundedWorldPos = glm::vec3(shadowMapRoundedWorldPos4D) / shadowMapRoundedWorldPos4D.w;
		
		data.lightViewProjection = _projection *
		                           glm::lookAt(
			                           shadowMapRoundedWorldPos,
			                           shadowMapRoundedWorldPos + getTransform().getDown(),
			                           getTransform().getForward());
		data.shadowMapTextureView = &_shadowMapView;
		data.shadowMapTexelWorldSize = texelWorldSize;
	}
	
	renderRegistry.addRenderRequest(data);
}

void DirectionalLight::onDrawUi()
{
	glm::vec3 imGuiSrgbColor = getSrgbColor();
	if (ImGui::ColorEdit3("Color", glm::value_ptr(imGuiSrgbColor)))
	{
		setSrgbColor(imGuiSrgbColor);
	}
	
	float intensity = getIntensity();
	if (ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp))
	{
		setIntensity(intensity);
	}
	
	float angularDiameter = getAngularDiameter();
	if (ImGui::DragFloat("Angular Diameter", &angularDiameter, 0.01f, 0.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
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
	
	_changed();
}

void DirectionalLight::deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot)
{
	setSrgbColor(glm::make_vec3(jsonRoot["color"].get<std::vector<float>>().data()));
	setIntensity(jsonRoot["intensity"].get<float>());
	setCastShadows(jsonRoot["cast_shadows"].get<bool>());
	setResolution(jsonRoot["shadow_resolution"].get<int>());
}

void DirectionalLight::deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot)
{
	setSrgbColor(glm::make_vec3(jsonRoot["color"].get<std::vector<float>>().data()));
	setIntensity(jsonRoot["intensity"].get<float>());
	setCastShadows(jsonRoot["cast_shadows"].get<bool>());
	setResolution(jsonRoot["shadow_resolution"].get<int>());
	setAngularDiameter(jsonRoot["angular_diameter"].get<float>());
}
