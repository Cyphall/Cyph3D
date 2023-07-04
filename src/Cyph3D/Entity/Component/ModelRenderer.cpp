#include "ModelRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Helper/ImGuiHelper.h"

#include <imgui.h>
#include <imgui_stdlib.h>

const char* ModelRenderer::identifier = "ModelRenderer";

ModelRenderer::ModelRenderer(Entity& entity):
Component(entity)
{
	setMaterialPath("materials/internal/Default Material/Default Material.c3dmaterial");
	setMeshPath("meshes/internal/Default Mesh/Default Mesh.obj");
}

const std::string* ModelRenderer::getMaterialPath() const
{
	return _materialPath.has_value() ? &_materialPath.value() : nullptr;
}

void ModelRenderer::setMaterialPath(std::optional<std::string_view> path)
{
	if (path)
	{
		_materialPath = *path;
		_material = Engine::getAssetManager().loadMaterial(path.value());
	}
	else
	{
		_materialPath = std::nullopt;
		_material = nullptr;
	}
}

MaterialAsset* ModelRenderer::getMaterial() const
{
	return _material;
}

const std::string* ModelRenderer::getMeshPath() const
{
	return _meshPath.has_value() ? &_meshPath.value() : nullptr;
}

void ModelRenderer::setMeshPath(std::optional<std::string_view> path)
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

MeshAsset* ModelRenderer::getMesh() const
{
	return _mesh;
}

bool ModelRenderer::getContributeShadows() const
{
	return _contributeShadows;
}

void ModelRenderer::setContributeShadows(bool contributeShadows)
{
	_contributeShadows = contributeShadows;
}

ObjectSerialization ModelRenderer::serialize() const
{
	ObjectSerialization serialization;
	serialization.version = 3;
	serialization.identifier = getIdentifier();
	
	const std::string* materialPath = getMaterialPath();
	if (materialPath)
	{
		serialization.data["material"] = *materialPath;
	}
	else
	{
		serialization.data["material"] = nullptr;
	}
	
	const std::string* meshPath = getMeshPath();
	if (meshPath)
	{
		serialization.data["mesh"] = *meshPath;
	}
	else
	{
		serialization.data["mesh"] = nullptr;
	}
	
	serialization.data["contribute_shadows"] = getContributeShadows();
	
	return serialization;
}

void ModelRenderer::deserialize(const ObjectSerialization& modelRendererSerialization)
{
	if (!modelRendererSerialization.data["material"].is_null())
	{
		if (modelRendererSerialization.version <= 1)
		{
			Logger::info("ModelRenderer deseralization: converting material identifier from version 1.");
			std::string oldName = modelRendererSerialization.data["material"].get<std::string>();
			std::string newFileName = std::filesystem::path(oldName).filename().generic_string();
			std::string convertedPath = std::format("materials/{}/{}.c3dmaterial", oldName, newFileName);
			if (std::filesystem::exists(FileHelper::getAssetDirectoryPath() / convertedPath))
			{
				setMaterialPath(convertedPath);
			}
			else
			{
				Logger::warning("ModelRenderer deseralization: unable to convert material identifier from version 1.");
			}
		}
		else
		{
			setMaterialPath(modelRendererSerialization.data["material"].get<std::string>());
		}
	}
	else
	{
		setMaterialPath(std::nullopt);
	}
	
	if (modelRendererSerialization.version <= 2)
	{
		ObjectSerialization shapeSerialization = ObjectSerialization::fromJson(modelRendererSerialization.data["shape"]);
		
		if (shapeSerialization.identifier == "MeshShape")
		{
			const nlohmann::ordered_json& jsonMeshPath = shapeSerialization.data["model"];
			if (!jsonMeshPath.is_null())
			{
				if (shapeSerialization.version <= 1)
				{
					Logger::info("ModelRenderer deseralization: converting mesh identifier from MeshShape version 1.");
					std::string oldName = jsonMeshPath.get<std::string>();
					std::string convertedPath = std::format("meshes/{}.obj", oldName);
					if (std::filesystem::exists(FileHelper::getAssetDirectoryPath() / convertedPath))
					{
						setMeshPath(convertedPath);
					}
					else
					{
						Logger::warning("ModelRenderer deseralization: unable to convert mesh identifier from MeshShape version 1.");
					}
				}
				else
				{
					setMeshPath(jsonMeshPath.get<std::string>());
				}
			}
			else
			{
				setMeshPath(std::nullopt);
			}
		}
		else if (shapeSerialization.identifier == "PlaneShape")
		{
			setMeshPath("meshes/plane.obj");
		}
		else if (shapeSerialization.identifier == "SphereShape")
		{
			setMeshPath("meshes/sphere.obj");
		}
	}
	else
	{
		if (!modelRendererSerialization.data["mesh"].is_null())
		{
			setMeshPath(modelRendererSerialization.data["mesh"].get<std::string>());
		}
		else
		{
			setMeshPath(std::nullopt);
		}
	}
	
	setContributeShadows(modelRendererSerialization.data["contribute_shadows"].get<bool>());
}

void ModelRenderer::onPreRender(RenderRegistry& renderRegistry, Camera& camera)
{
	MaterialAsset* material;
	if (!_material)
	{
		material = MaterialAsset::getMissingMaterial();
	}
	else if (!_material->isLoaded())
	{
		material = MaterialAsset::getDefaultMaterial();
	}
	else
	{
		material = _material;
	}
	
	MeshAsset* mesh;
	if (!_mesh)
	{
		mesh = MeshAsset::getMissingMesh();
	}
	else if (!_mesh->isLoaded())
	{
		mesh = MeshAsset::getDefaultMesh();
	}
	else
	{
		mesh = _mesh;
	}
	
	if (material->isLoaded() && mesh->isLoaded())
	{
		RenderData data{};
		data.material = material;
		data.mesh = mesh;
		data.owner = &getEntity();
		data.contributeShadows = getContributeShadows();
		data.matrix = getTransform().getLocalToWorldMatrix();
		
		renderRegistry.addRenderRequest(data);
	}
}

void ModelRenderer::onDrawUi()
{
	std::optional<std::string_view> newMaterialPath;
	if (ImGuiHelper::AssetInputWidget(getMaterialPath(), "Material", "asset_material", newMaterialPath))
	{
		setMaterialPath(newMaterialPath);
	}
	
	std::optional<std::string_view> newMeshPath;
	if (ImGuiHelper::AssetInputWidget(getMeshPath(), "Mesh", "asset_mesh", newMeshPath))
	{
		setMeshPath(newMeshPath);
	}
	
	bool contributeShadows = getContributeShadows();
	if (ImGui::Checkbox("Contribute Shadows", &contributeShadows))
	{
		setContributeShadows(contributeShadows);
	}
}

const char* ModelRenderer::getIdentifier() const
{
	return identifier;
}

void ModelRenderer::duplicate(Entity& targetEntity) const
{
	ModelRenderer& newComponent = targetEntity.addComponent<ModelRenderer>();
	if (_materialPath)
	{
		newComponent.setMaterialPath(_materialPath.value());
	}
	if (_meshPath)
	{
		newComponent.setMeshPath(_meshPath.value());
	}
	newComponent.setContributeShadows(getContributeShadows());
}