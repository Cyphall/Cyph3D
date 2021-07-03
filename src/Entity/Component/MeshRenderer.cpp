#include <imgui.h>
#include <imgui_stdlib.h>
#include "MeshRenderer.h"
#include "../Entity.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Rendering/Renderer.h"

const char* MeshRenderer::identifier = "MeshRenderer";

MeshRenderer::MeshRenderer(Entity& entity):
Component(entity)
{
	setMaterial(Material::getDefault());
}

Material* MeshRenderer::getMaterial() const
{
	return _material;
}

void MeshRenderer::setMaterial(Material* material)
{
	_material = material;
}

Model* MeshRenderer::getModel() const
{
	return _model;
}

void MeshRenderer::setModel(Model* model)
{
	_model = model;
}

bool MeshRenderer::getContributeShadows() const
{
	return _contributeShadows;
}

void MeshRenderer::setContributeShadows(bool contributeShadows)
{
	_contributeShadows = contributeShadows;
}

ComponentSerialization MeshRenderer::serialize() const
{
	ComponentSerialization serialization;
	serialization.version = 1;
	serialization.identifier = getIdentifier();
	
	Model* model = getModel();
	if (model)
	{
		serialization.data["model"] = model->getName();
	}
	else
	{
		serialization.data["model"] = nullptr;
	}
	
	Material* material = getMaterial();
	if (material)
	{
		serialization.data["material"] = material->getName();
	}
	else
	{
		serialization.data["material"] = nullptr;
	}
	
	serialization.data["contribute_shadows"] = getContributeShadows();
	
	return serialization;
}

void MeshRenderer::deserialize(const ComponentSerialization& serialization)
{
	Scene& scene = getEntity().getScene();
	
	if (!serialization.data["material"].is_null())
	{
		setMaterial(scene.getRM().requestMaterial(serialization.data["material"].get<std::string>()));
	}
	
	if (!serialization.data["model"].is_null())
	{
		setModel(scene.getRM().requestModel(serialization.data["model"].get<std::string>()));
	}
	
	setContributeShadows(serialization.data["contribute_shadows"].get<bool>());
}

Material* MeshRenderer::getDrawingMaterial()
{
	return _material != nullptr ? _material : Material::getMissing();
}

void MeshRenderer::onPreRender(RenderContext& context)
{
	Model* model = getModel();
	
	if (model == nullptr || !model->isResourceReady())
		return;
	
	RenderData data;
	data.material = getDrawingMaterial();
	data.mesh = &model->getResource();
	data.owner = &getEntity();
	data.contributeShadows = getContributeShadows();
	data.matrix = getTransform().getLocalToWorldMatrix();
	
	context.renderer.requestMeshRendering(data);
}

void MeshRenderer::onDrawUi()
{
	Model* model = getModel();
	std::string modelName = model != nullptr ? model->getName() : "None";
	ImGui::InputText("Mesh", &modelName, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MeshDragDrop");
		if (payload)
		{
			setModel(Engine::getScene().getRM().requestModel(*(*static_cast<const std::string**>(payload->Data))));
		}
		ImGui::EndDragDropTarget();
	}
	
	Material* material = getDrawingMaterial();
	std::string materialName = material->getName();
	ImGui::InputText("Material", &materialName, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MaterialDragDrop");
		if (payload)
		{
			std::string newMaterialName = *(*static_cast<const std::string**>(payload->Data));
			if (newMaterialName == "internal/Default Material")
			{
				setMaterial(Material::getDefault());
			}
			else
			{
				setMaterial(Engine::getScene().getRM().requestMaterial(newMaterialName));
			}
		}
		ImGui::EndDragDropTarget();
	}
	
	bool contributeShadows = getContributeShadows();
	if (ImGui::Checkbox("Contribute Shadows", &contributeShadows))
	{
		setContributeShadows(contributeShadows);
	}
}

const char* MeshRenderer::getIdentifier() const
{
	return identifier;
}

void MeshRenderer::duplicate(Entity& targetEntity) const
{
	MeshRenderer& newComponent = targetEntity.addComponent<MeshRenderer>();
	newComponent.setMaterial(getMaterial());
	newComponent.setModel(getModel());
	newComponent.setContributeShadows(getContributeShadows());
}
