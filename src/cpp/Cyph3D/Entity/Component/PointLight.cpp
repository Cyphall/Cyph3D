#include "PointLight.h"

#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Scene/Scene.h"

#include <glm/gtc/integer.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

const char* const c3d::PointLight::identifier = "PointLight";

c3d::PointLight::PointLight(Entity& entity):
	LightBase(entity)
{
}

void c3d::PointLight::onPreRender(RenderRegistry& renderRegistry, Camera& camera)
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

void c3d::PointLight::onDrawUi()
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

	float radius = getRadius();
	if (ImGui::DragFloat("Radius", &radius, 0.01f, 0.0f, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp))
	{
		setRadius(radius);
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

const char* c3d::PointLight::getIdentifier() const
{
	return identifier;
}

bool c3d::PointLight::getCastShadows() const
{
	return _castShadows;
}

void c3d::PointLight::setCastShadows(bool value)
{
	_castShadows = value;

	_changed();
}

uint32_t c3d::PointLight::getResolution() const
{
	return _resolution;
}

void c3d::PointLight::setResolution(uint32_t value)
{
	_resolution = value;

	_changed();
}

float c3d::PointLight::getRadius() const
{
	return _radius;
}

void c3d::PointLight::setRadius(float value)
{
	_radius = value;

	_changed();
}

void c3d::PointLight::duplicate(Entity& targetEntity) const
{
	PointLight& newComponent = targetEntity.addComponent<PointLight>();
	newComponent.setSrgbColor(getSrgbColor());
	newComponent.setIntensity(getIntensity());
	newComponent.setCastShadows(getCastShadows());
	newComponent.setResolution(getResolution());
	newComponent.setRadius(getRadius());
}

c3d::ObjectSerialization c3d::PointLight::serialize() const
{
	ObjectSerialization serialization;
	serialization.version = 2;
	serialization.identifier = getIdentifier();

	glm::vec3 color = getSrgbColor();
	serialization.data["color"] = {color.r, color.g, color.b};
	serialization.data["intensity"] = getIntensity();
	serialization.data["cast_shadows"] = getCastShadows();
	serialization.data["shadow_resolution"] = getResolution();
	serialization.data["radius"] = getRadius();

	return serialization;
}

void c3d::PointLight::deserialize(const ObjectSerialization& serialization)
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

void c3d::PointLight::deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot)
{
	setSrgbColor(glm::make_vec3(jsonRoot["color"].get<std::vector<float>>().data()));
	setIntensity(jsonRoot["intensity"].get<float>());
	setCastShadows(jsonRoot["cast_shadows"].get<bool>());
	setResolution(jsonRoot["shadow_resolution"].get<int>());
}

void c3d::PointLight::deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot)
{
	setSrgbColor(glm::make_vec3(jsonRoot["color"].get<std::vector<float>>().data()));
	setIntensity(jsonRoot["intensity"].get<float>());
	setCastShadows(jsonRoot["cast_shadows"].get<bool>());
	setResolution(jsonRoot["shadow_resolution"].get<int>());
	setRadius(jsonRoot["radius"].get<float>());
}