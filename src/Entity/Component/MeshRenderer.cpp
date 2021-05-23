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
	ComponentSerialization data(1);
	
	Model* model = getModel();
	if (model)
	{
		data.json["model"] = model->getName();
	}
	else
	{
		data.json["model"] = nullptr;
	}
	
	Material* material = getMaterial();
	if (model)
	{
		data.json["material"] = material->getName();
	}
	else
	{
		data.json["material"] = nullptr;
	}
	
	data.json["contribute_shadows"] = getContributeShadows();
	
	return data;
}

void MeshRenderer::deserialize(const ComponentSerialization& data)
{
	Scene& scene = getEntity().getScene();
	
	if (!data.json["material"].is_null())
	{
		setMaterial(scene.getRM().requestMaterial(data.json["material"].get<std::string>()));
	}
	
	if (!data.json["model"].is_null())
	{
		setModel(scene.getRM().requestModel(data.json["model"].get<std::string>()));
	}
	
	setContributeShadows(data.json["contribute_shadows"].get<bool>());
}

Material* MeshRenderer::getDrawingMaterial()
{
	return _material != nullptr ? _material : Material::getMissing();
}

void MeshRenderer::onPreRender()
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
	
	Engine::getRenderer().requestMeshRendering(data);
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
