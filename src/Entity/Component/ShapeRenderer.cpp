#include <imgui.h>
#include <imgui_stdlib.h>
#include "ShapeRenderer.h"
#include "../Entity.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Rendering/Renderer.h"

const char* ShapeRenderer::identifier = "ShapeRenderer";

ShapeRenderer::ShapeRenderer(Entity& entity):
Component(entity)
{
	setMaterial(Material::getDefault());
}

Material* ShapeRenderer::getMaterial() const
{
	return _material;
}

void ShapeRenderer::setMaterial(Material* material)
{
	_material = material;
}

Model* ShapeRenderer::getModel() const
{
	return _model;
}

void ShapeRenderer::setModel(Model* model)
{
	_model = model;
}

bool ShapeRenderer::getContributeShadows() const
{
	return _contributeShadows;
}

void ShapeRenderer::setContributeShadows(bool contributeShadows)
{
	_contributeShadows = contributeShadows;
}

ObjectSerialization ShapeRenderer::serialize() const
{
	ObjectSerialization serialization;
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

void ShapeRenderer::deserialize(const ObjectSerialization& serialization)
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

Material* ShapeRenderer::getDrawingMaterial()
{
	return _material != nullptr ? _material : Material::getMissing();
}

void ShapeRenderer::onPreRender(RenderContext& context)
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
	
	context.renderer.requestShapeRendering(data);
}

void ShapeRenderer::onDrawUi()
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

const char* ShapeRenderer::getIdentifier() const
{
	return identifier;
}

void ShapeRenderer::duplicate(Entity& targetEntity) const
{
	ShapeRenderer& newComponent = targetEntity.addComponent<ShapeRenderer>();
	newComponent.setMaterial(getMaterial());
	newComponent.setModel(getModel());
	newComponent.setContributeShadows(getContributeShadows());
}
