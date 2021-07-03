#include "DirectionalLight.h"
#include "../Entity.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/integer.hpp>
#include <imgui.h>
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Rendering/Renderer.h"
#include "../../Rendering/RenderRegistry.h"

const char* DirectionalLight::identifier = "DirectionalLight";
glm::mat4 DirectionalLight::_projection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, -50.0f, 50.0f);

DirectionalLight::DirectionalLight(Entity& entity):
LightBase(entity)
{
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/shadow mapping/directional light");
	
	_shadowMapProgram = getEntity().getScene().getRM().requestShaderProgram(createInfo);
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

void DirectionalLight::updateShadowMap(VertexArray& vao, RenderRegistry& registry)
{
	if (!_castShadows) return;
	
	glViewport(0, 0, _resolution, _resolution);
	
	_shadowMapFb->bindForDrawing();
	_shadowMapProgram->bind();
	
	float depthColor = 1;
	_shadowMap->clear(GL_DEPTH_COMPONENT, GL_FLOAT, &depthColor);
	
	for (auto& meshRenderer : registry.meshes)
	{
		if (!meshRenderer.contributeShadows) continue;
		
		const Buffer<Mesh::VertexData>& vbo = meshRenderer.mesh->getVBO();
		const Buffer<GLuint>& ibo = meshRenderer.mesh->getIBO();
		vao.bindBufferToSlot(vbo, 0);
		vao.bindIndexBuffer(ibo);
		
		glm::mat4 mvp = _viewProjection * meshRenderer.matrix;
		
		_shadowMapProgram->setUniform("u_mvp", mvp);
		
		glDrawElements(GL_TRIANGLES, ibo.getCount(), GL_UNSIGNED_INT, nullptr);
	}
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

ComponentSerialization DirectionalLight::serialize() const
{
	ComponentSerialization serialization;
	
	serialization.version = 1;
	
	glm::vec3 color = getSrgbColor();
	serialization.data["color"] = {color.r, color.g, color.b};
	serialization.data["intensity"] = getIntensity();
	serialization.data["cast_shadows"] = getCastShadows();
	serialization.data["shadow_resolution"] = getResolution();
	
	return serialization;
}

void DirectionalLight::deserialize(const ComponentSerialization& serialization)
{
	setSrgbColor(glm::make_vec3(serialization.data["color"].get<std::vector<float>>().data()));
	setIntensity(serialization.data["intensity"].get<float>());
	setCastShadows(serialization.data["cast_shadows"].get<bool>());
	setResolution(serialization.data["shadow_resolution"].get<int>());
}

void DirectionalLight::onPreRender(RenderContext& context)
{
	if (getCastShadows())
	{
		Scene& scene = getEntity().getScene();
		_viewProjection = _projection *
						  glm::lookAt(
							  context.camera.getPosition(),
							  context.camera.getPosition() + getLightDirection(),
							  glm::vec3(0, 1, 0));
	}
	
	RenderData data;
	data.nativeData = NativeData
	{
		.fragToLightDirection = -getLightDirection(),
		.intensity = getIntensity(),
		.color = getLinearColor(),
		.castShadows = _castShadows,
		.lightViewProjection = _castShadows ? _viewProjection : glm::mat4(),
		.shadowMap = _castShadows ? _shadowMap->getBindlessHandle() : 0,
		.mapSize = _mapSize,
		.mapDepth = _mapDepth,
	};
	data.light = this;
	
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
}
