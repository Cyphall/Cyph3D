#include "PointLight.h"
#include "../Entity.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>
#include <imgui.h>
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Rendering/Renderer.h"
#include "../../Rendering/RenderRegistry.h"

const char* PointLight::identifier = "PointLight";
glm::mat4 PointLight::_projection = glm::perspective(glm::radians(90.0f), 1.0f, _NEAR, _FAR);

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

void PointLight::updateShadowMap(VertexArray& vao, RenderRegistry& registry)
{
	if (!_castShadows) return;
	
	glViewport(0, 0, _resolution, _resolution);
	
	glm::vec3 worldPos = getTransform().getWorldPosition();
	
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
	
	_shadowMapFb->bindForDrawing();
	_shadowMapProgram->bind();
	_shadowMapProgram->setUniform("u_viewProjections", _viewProjections, 6);
	_shadowMapProgram->setUniform("u_lightPos", worldPos);
	_shadowMapProgram->setUniform("u_far", _FAR);
	
	float depthColor = 1;
	_shadowMap->clear(GL_DEPTH_COMPONENT, GL_FLOAT, &depthColor);
	
	for (auto& meshRenderer : registry.meshes)
	{
		if (!meshRenderer.contributeShadows) continue;
		
		const Buffer<Mesh::VertexData>& vbo = meshRenderer.mesh->getVBO();
		const Buffer<GLuint>& ibo = meshRenderer.mesh->getIBO();
		vao.bindBufferToSlot(vbo, 0);
		vao.bindIndexBuffer(ibo);
		
		_shadowMapProgram->setUniform("u_model", meshRenderer.matrix);
		
		glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
	}
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

ComponentSerialization PointLight::serialize() const
{
	ComponentSerialization data(1);
	
	glm::vec3 color = getSrgbColor();
	data.json["color"] = {color.r, color.g, color.b};
	data.json["intensity"] = getIntensity();
	data.json["cast_shadows"] = getCastShadows();
	data.json["shadow_resolution"] = getResolution();
	
	return data;
}

void PointLight::deserialize(const ComponentSerialization& data)
{
	setSrgbColor(glm::make_vec3(data.json["color"].get<std::vector<float>>().data()));
	setIntensity(data.json["intensity"].get<float>());
	setCastShadows(data.json["cast_shadows"].get<bool>());
	setResolution(data.json["shadow_resolution"].get<int>());
}

void PointLight::onPreRender(RenderContext& context)
{
	RenderData data;
	data.nativeData = NativeData
	{
		.pos = getTransform().getWorldPosition(),
		.intensity = getIntensity(),
		.color = getLinearColor(),
		.castShadows = getCastShadows(),
		.shadowMap = getCastShadows() ? _shadowMap->getBindlessHandle() : 0,
		._far = getCastShadows() ? _FAR : 0,
		.maxTexelSizeAtUnitDistance = 2.0f / getResolution()
	};
	data.light = this;
	
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
}
