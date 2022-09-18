#include "MeshShape.h"

#include "Cyph3D/Entity/Component/ShapeRenderer.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/ResourceManagement/Model.h"
#include "Cyph3D/Scene/Scene.h"

#include <imgui.h>
#include <imgui_stdlib.h>

const char* MeshShape::identifier = "MeshShape";

MeshShape::MeshShape(ShapeRenderer& shapeRenderer):
Shape(shapeRenderer)
{

}

const char* MeshShape::getIdentifier() const
{
	return identifier;
}

void MeshShape::duplicate(ShapeRenderer& targetShapeRenderer) const
{
	MeshShape& newShape = targetShapeRenderer.setShape<MeshShape>();
	newShape.setModel(getModel());
}

ObjectSerialization MeshShape::serialize() const
{
	ObjectSerialization serialization;
	serialization.version = 1;
	serialization.identifier = getIdentifier();
	
	const Model* model = getModel();
	if (model)
	{
		serialization.data["model"] = model->getName();
	}
	else
	{
		serialization.data["model"] = nullptr;
	}
	
	return serialization;
}

void MeshShape::deserialize(const ObjectSerialization& serialization)
{
	Scene& scene = getShapeRenderer().getEntity().getScene();
	
	if (!serialization.data["model"].is_null())
	{
		setModel(scene.getRM().requestModel(serialization.data["model"].get<std::string>()));
	}
}

const Model* MeshShape::getModel()
{
	return _model;
}

const Model* MeshShape::getModel() const
{
	return _model;
}

void MeshShape::setModel(const Model* model)
{
	_model = model;
}

void MeshShape::onDrawUi()
{
	const Model* model = getModel();
	std::string modelName = model != nullptr ? model->getName() : "None";
	ImGui::InputText("Model", &modelName, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MeshDragDrop");
		if (payload)
		{
			setModel(getShapeRenderer().getEntity().getScene().getRM().requestModel(*(*static_cast<const std::string**>(payload->Data))));
		}
		ImGui::EndDragDropTarget();
	}
}

bool MeshShape::isReadyForRasterisationRender() const
{
	const Model* model = getModel();
	
	return model != nullptr && model->isResourceReady();
}

bool MeshShape::isReadyForRaytracingRender() const
{
	const Model* model = getModel();
	
	return model != nullptr && model->isResourceReady();
}

const Mesh& MeshShape::getMeshToRender() const
{
	return getModel()->getResource();
}