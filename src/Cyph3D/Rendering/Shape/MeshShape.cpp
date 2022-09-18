#include "MeshShape.h"

#include "Cyph3D/Entity/Component/ShapeRenderer.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/ResourceManagement/Model.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Helper/FileHelper.h"

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
	serialization.version = 2;
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
		if (serialization.version <= 1)
		{
			Logger::info("MeshShape deseralization: converting model identifier from version 1.");
			std::string oldName = serialization.data["model"].get<std::string>();
			std::string convertedPath = std::format("meshes/{}.obj", oldName);
			if (std::filesystem::exists(FileHelper::getResourcePath() / convertedPath))
			{
				setModel(scene.getRM().requestModel(convertedPath));
			}
			else
			{
				Logger::warning("MeshShape deseralization: unable to convert model identifier from version 1.");
			}
		}
		else
		{
			setModel(scene.getRM().requestModel(serialization.data["model"].get<std::string>()));
		}
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
	std::string modelPath = model != nullptr ? model->getName() : "None";
	ImGui::InputText("Model", &modelPath, ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset_model");
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