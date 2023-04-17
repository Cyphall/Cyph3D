#include "MeshShape.h"

#include "Cyph3D/Entity/Component/ShapeRenderer.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/Asset/RuntimeAsset/ModelAsset.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Helper/ImGuiHelper.h"

#include <imgui.h>
#include <imgui_stdlib.h>

const char* MeshShape::identifier = "MeshShape";

MeshShape::MeshShape(ShapeRenderer& shapeRenderer):
Shape(shapeRenderer)
{

}

const std::string* MeshShape::getModelPath() const
{
	return _modelPath.has_value() ? &_modelPath.value() : nullptr;
}

void MeshShape::setModelPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_modelPath = *path;
		_model = Engine::getAssetManager().loadModel(*path);
	}
	else
	{
		_modelPath = std::nullopt;
		_model = nullptr;
	}
}

ModelAsset* MeshShape::getModel() const
{
	return _model;
}

bool MeshShape::isReadyForRasterisationRender() const
{
	ModelAsset* model = getModel();

	return model != nullptr && model->isLoaded();
}

bool MeshShape::isReadyForRaytracingRender() const
{
	ModelAsset* model = getModel();

	return model != nullptr && model->isLoaded();
}

const VKPtr<VKBuffer<VertexData>>& MeshShape::getVertexBuffer() const
{
	return getModel()->getVertexBuffer();
}

const VKPtr<VKBuffer<uint32_t>>& MeshShape::getIndexBuffer() const
{
	return getModel()->getIndexBuffer();
}

void MeshShape::onDrawUi()
{
	std::optional<std::string_view> newPath;
	if (ImGuiHelper::AssetInputWidget(getModelPath(), "Model", "asset_mesh", newPath))
	{
		setModelPath(newPath);
	}
}

const char* MeshShape::getIdentifier() const
{
	return identifier;
}

void MeshShape::duplicate(ShapeRenderer& targetShapeRenderer) const
{
	MeshShape& newShape = targetShapeRenderer.setShape<MeshShape>();
	if (_modelPath)
	{
		newShape.setModelPath(_modelPath.value());
	}
}

ObjectSerialization MeshShape::serialize() const
{
	ObjectSerialization serialization;
	serialization.version = 2;
	serialization.identifier = getIdentifier();
	
	const std::string* modelPath = getModelPath();
	if (modelPath)
	{
		serialization.data["model"] = *modelPath;
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
	
	const nlohmann::ordered_json& jsonModelPath = serialization.data["model"];
	if (!jsonModelPath.is_null())
	{
		if (serialization.version <= 1)
		{
			Logger::info("MeshShape deseralization: converting model identifier from version 1.");
			std::string oldName = jsonModelPath.get<std::string>();
			std::string convertedPath = std::format("meshes/{}.obj", oldName);
			if (std::filesystem::exists(FileHelper::getAssetDirectoryPath() / convertedPath))
			{
				setModelPath(convertedPath);
			}
			else
			{
				Logger::warning("MeshShape deseralization: unable to convert model identifier from version 1.");
			}
		}
		else
		{
			setModelPath(jsonModelPath.get<std::string>());
		}
	}
}