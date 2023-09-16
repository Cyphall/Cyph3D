#include "DirectionalLight.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Scene/Scene.h"

#include <glm/gtc/integer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

const char* const DirectionalLight::identifier = "DirectionalLight";

DirectionalLight::DirectionalLight(Entity& entity):
	LightBase(entity)
{

}

void DirectionalLight::onPreRender(RenderRegistry& renderRegistry, Camera& camera)
{
	RenderData data{
		.transform = getTransform(),
		.intensity = getIntensity(),
		.color = getLinearColor(),
		.castShadows = getCastShadows(),
		.shadowMapResolution = getResolution()
	};
	
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

bool DirectionalLight::getCastShadows() const
{
	return _castShadows;
}

void DirectionalLight::setCastShadows(bool value)
{
	_castShadows = value;
	
	_changed();
}

uint32_t DirectionalLight::getResolution() const
{
	return _resolution;
}

void DirectionalLight::setResolution(uint32_t value)
{
	_resolution = value;
	
	_changed();
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

void DirectionalLight::duplicate(Entity& targetEntity) const
{
	DirectionalLight& newComponent = targetEntity.addComponent<DirectionalLight>();
	newComponent.setSrgbColor(getSrgbColor());
	newComponent.setIntensity(getIntensity());
	newComponent.setCastShadows(getCastShadows());
	newComponent.setResolution(getResolution());
	newComponent.setAngularDiameter(getAngularDiameter());
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
