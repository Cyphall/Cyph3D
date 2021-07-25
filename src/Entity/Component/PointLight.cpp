#include "PointLight.h"
#include "../Entity.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>
#include <imgui.h>
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Rendering/Renderer/Renderer.h"
#include "../../Rendering/RenderRegistry.h"

const char* PointLight::identifier = "PointLight";
glm::mat4 PointLight::_projection = glm::perspective(glm::radians(90.0f), 1.0f, NEAR_DISTANCE, FAR_DISTANCE);

PointLight::PointLight(Entity& entity):
LightBase(entity)
{
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_GEOMETRY_SHADER].emplace_back("internal/shadow mapping/point light");
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/shadow mapping/point light");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back("internal/shadow mapping/point light");
	
	_shadowMapProgram = getEntity().getScene().getRM().requestShaderProgram(createInfo);
}

void PointLight::setCastShadows(bool value)
{
	if (_castShadows == value) return;
	
	_castShadows = value;
	
	if (value)
	{
		_shadowMapFb = std::make_unique<Framebuffer>(glm::ivec2(_resolution));
		
		CubemapCreateInfo createInfo
			{
				.size = _shadowMapFb->getSize(),
				.internalFormat = GL_DEPTH_COMPONENT32
			};
		_shadowMap = std::make_unique<Cubemap>(createInfo);
		
		_shadowMapFb->attachDepth(*_shadowMap.get());
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

void PointLight::setResolution(int value)
{
	bool castShadows = getCastShadows();
	
	if (castShadows)
		setCastShadows(false);
	
	_resolution = value;
	
	if (castShadows)
		setCastShadows(true);
}

int PointLight::getResolution() const
{
	return _resolution;
}

ObjectSerialization PointLight::serialize() const
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

void PointLight::deserialize(const ObjectSerialization& serialization)
{
	setSrgbColor(glm::make_vec3(serialization.data["color"].get<std::vector<float>>().data()));
	setIntensity(serialization.data["intensity"].get<float>());
	setCastShadows(serialization.data["cast_shadows"].get<bool>());
	setResolution(serialization.data["shadow_resolution"].get<int>());
	
	if (serialization.version >= 2)
	{
		setRadius(serialization.data["radius"].get<float>());
	}
}

void PointLight::onPreRender(RenderContext& context)
{
	RenderData data;
	data.pos = getTransform().getWorldPosition();
	data.intensity = getIntensity();
	data.color = getLinearColor();
	data.radius = getRadius();
	data.castShadows = _castShadows;
	if (_castShadows)
	{
		data.viewProjections[0] = _projection *
							  glm::lookAt(data.pos, data.pos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		data.viewProjections[1] = _projection *
							  glm::lookAt(data.pos, data.pos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		data.viewProjections[2] = _projection *
							  glm::lookAt(data.pos, data.pos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		data.viewProjections[3] = _projection *
							  glm::lookAt(data.pos, data.pos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		data.viewProjections[4] = _projection *
							  glm::lookAt(data.pos, data.pos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		data.viewProjections[5] = _projection *
							  glm::lookAt(data.pos, data.pos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
		data.shadowMapTexture = _shadowMap.get();
		data.shadowMapFramebuffer = _shadowMapFb.get();
		data.mapResolution = getResolution();
		data.far = FAR_DISTANCE;
	}
	
	context.renderer.requestLightRendering(data);
}

void PointLight::onDrawUi()
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
	
	float radius = getRadius();
	if (ImGui::SliderFloat("Radius", &radius, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
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

const char* PointLight::getIdentifier() const
{
	return identifier;
}

void PointLight::duplicate(Entity& targetEntity) const
{
	PointLight& newComponent = targetEntity.addComponent<PointLight>();
	newComponent.setSrgbColor(getSrgbColor());
	newComponent.setIntensity(getIntensity());
	newComponent.setCastShadows(getCastShadows());
	newComponent.setResolution(getResolution());
	newComponent.setRadius(getRadius());
}

float PointLight::getRadius() const
{
	return _radius;
}

void PointLight::setRadius(float value)
{
	_radius = value;
}
