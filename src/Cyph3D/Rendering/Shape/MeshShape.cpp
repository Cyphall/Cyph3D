#include "MeshShape.h"

#include "Cyph3D/Entity/Component/ShapeRenderer.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
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

const std::string* MeshShape::getMeshPath() const
{
	return _meshPath.has_value() ? &_meshPath.value() : nullptr;
}

void MeshShape::setMeshPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_meshPath = *path;
		_mesh = Engine::getAssetManager().loadMesh(*path);
	}
	else
	{
		_meshPath = std::nullopt;
		_mesh = nullptr;
	}
}

MeshAsset* MeshShape::getMesh() const
{
	return _mesh;
}

bool MeshShape::isReadyForRasterisationRender() const
{
	MeshAsset* mesh = getMesh();

	return mesh != nullptr && mesh->isLoaded();
}

bool MeshShape::isReadyForRaytracingRender() const
{
	MeshAsset* mesh = getMesh();

	return mesh != nullptr && mesh->isLoaded();
}

const VKPtr<VKBuffer<VertexData>>& MeshShape::getVertexBuffer() const
{
	return getMesh()->getVertexBuffer();
}

const VKPtr<VKBuffer<uint32_t>>& MeshShape::getIndexBuffer() const
{
	return getMesh()->getIndexBuffer();
}

void MeshShape::onDrawUi()
{
	std::optional<std::string_view> newPath;
	if (ImGuiHelper::AssetInputWidget(getMeshPath(), "Mesh", "asset_mesh", newPath))
	{
		setMeshPath(newPath);
	}
}

const char* MeshShape::getIdentifier() const
{
	return identifier;
}

void MeshShape::duplicate(ShapeRenderer& targetShapeRenderer) const
{
	MeshShape& newShape = targetShapeRenderer.setShape<MeshShape>();
	if (_meshPath)
	{
		newShape.setMeshPath(_meshPath.value());
	}
}

ObjectSerialization MeshShape::serialize() const
{
	ObjectSerialization serialization;
	serialization.version = 2;
	serialization.identifier = getIdentifier();
	
	const std::string* meshPath = getMeshPath();
	if (meshPath)
	{
		serialization.data["model"] = *meshPath;
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
	
	const nlohmann::ordered_json& jsonMeshPath = serialization.data["model"];
	if (!jsonMeshPath.is_null())
	{
		if (serialization.version <= 1)
		{
			Logger::info("MeshShape deseralization: converting mesh identifier from version 1.");
			std::string oldName = jsonMeshPath.get<std::string>();
			std::string convertedPath = std::format("meshes/{}.obj", oldName);
			if (std::filesystem::exists(FileHelper::getAssetDirectoryPath() / convertedPath))
			{
				setMeshPath(convertedPath);
			}
			else
			{
				Logger::warning("MeshShape deseralization: unable to convert mesh identifier from version 1.");
			}
		}
		else
		{
			setMeshPath(jsonMeshPath.get<std::string>());
		}
	}
}