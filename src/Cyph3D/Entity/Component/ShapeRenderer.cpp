#include "ShapeRenderer.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/RenderContext.h"
#include "Cyph3D/Rendering/Renderer/Renderer.h"
#include "Cyph3D/Rendering/Shape/MeshShape.h"
#include "Cyph3D/Rendering/Shape/PlaneShape.h"
#include "Cyph3D/Rendering/Shape/SphereShape.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Asset/AssetManager.h"

#include <imgui.h>
#include <imgui_stdlib.h>

const char* ShapeRenderer::identifier = "ShapeRenderer";
std::map<std::string, std::function<Shape&(ShapeRenderer&)>> ShapeRenderer::_allocators;

ShapeRenderer::ShapeRenderer(Entity& entity):
Component(entity), _shape(new MeshShape(*this)), _selectedShape(MeshShape::identifier)
{
	setMaterialPath("materials/internal/Default Material/Default Material.c3dmaterial");
}

const std::string* ShapeRenderer::getMaterialPath() const
{
	return _materialPath.has_value() ? &_materialPath.value() : nullptr;
}

void ShapeRenderer::setMaterialPath(std::optional<std::string_view> path)
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

MaterialAsset* ShapeRenderer::getMaterial() const
{
	return _material;
}

Shape& ShapeRenderer::getShape()
{
	return *_shape;
}

const Shape& ShapeRenderer::getShape() const
{
	return *_shape;
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
	serialization.version = 2;
	serialization.identifier = getIdentifier();
	
	serialization.data["shape"] = getShape().serialize().toJson();
	
	const std::string* materialPath = getMaterialPath();
	if (materialPath)
	{
		serialization.data["material"] = *materialPath;
	}
	else
	{
		serialization.data["material"] = nullptr;
	}
	
	serialization.data["contribute_shadows"] = getContributeShadows();
	
	return serialization;
}

void ShapeRenderer::deserialize(const ObjectSerialization& shapeRendererSerialization)
{
	Scene& scene = getEntity().getScene();
	
	if (!shapeRendererSerialization.data["material"].is_null())
	{
		if (shapeRendererSerialization.version <= 1)
		{
			Logger::info("ShapeRenderer deseralization: converting material identifier from version 1.");
			std::string oldName = shapeRendererSerialization.data["material"].get<std::string>();
			std::string newFileName = std::filesystem::path(oldName).filename().generic_string();
			std::string convertedPath = std::format("materials/{}/{}.c3dmaterial", oldName, newFileName);
			if (std::filesystem::exists(FileHelper::getAssetDirectoryPath() / convertedPath))
			{
				setMaterialPath(convertedPath);
			}
			else
			{
				Logger::warning("ShapeRenderer deseralization: unable to convert material identifier from version 1.");
			}
		}
		else
		{
			setMaterialPath(shapeRendererSerialization.data["material"].get<std::string>());
		}
	}

	ObjectSerialization shapeSerialization = ObjectSerialization::fromJson(shapeRendererSerialization.data["shape"]);
	Shape& shape = setShapeByIdentifier(shapeSerialization.identifier);
	shape.deserialize(shapeSerialization);
	
	_selectedShape = getShape().getIdentifier();
	
	setContributeShadows(shapeRendererSerialization.data["contribute_shadows"].get<bool>());
}

void ShapeRenderer::onPreRender(RenderContext& context)
{
	RenderData data{};
	data.material = getMaterial();
	data.shape = &getShape();
	data.owner = &getEntity();
	data.contributeShadows = getContributeShadows();
	data.matrix = getTransform().getLocalToWorldMatrix();
	
	context.renderer.requestShapeRendering(data);
}

void ShapeRenderer::onDrawUi()
{
	const char* materialPath = _materialPath.has_value() ? _materialPath.value().c_str() : "None";

	// Field is read-only anyway, we can safely remove the const from skyboxName
	ImGui::InputText("Material", const_cast<char*>(materialPath), ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("asset_material");
		if (payload)
		{
			setMaterialPath(*(*static_cast<const std::string**>(payload->Data)));
		}
		ImGui::EndDragDropTarget();
	}
	
	bool contributeShadows = getContributeShadows();
	if (ImGui::Checkbox("Contribute Shadows", &contributeShadows))
	{
		setContributeShadows(contributeShadows);
	}
	
	if (ImGui::BeginCombo("Shape", _selectedShape.c_str()))
	{
		for (auto it = _allocators.begin(); it != _allocators.end(); it++)
		{
			const bool is_selected = (_selectedShape == it->first);
			if (ImGui::Selectable(it->first.c_str(), is_selected))
			{
				_selectedShape = it->first;
				setShapeByIdentifier(_selectedShape);
			}
			
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	
	getShape().onDrawUi();
}

const char* ShapeRenderer::getIdentifier() const
{
	return identifier;
}

void ShapeRenderer::duplicate(Entity& targetEntity) const
{
	ShapeRenderer& newComponent = targetEntity.addComponent<ShapeRenderer>();
	if (_materialPath)
	{
		newComponent.setMaterialPath(_materialPath.value());
	}
	getShape().duplicate(newComponent);
	newComponent.setContributeShadows(getContributeShadows());
}

void ShapeRenderer::initAllocators()
{
	_allocators[MeshShape::identifier] = [](ShapeRenderer& shapeRenderer) -> decltype(auto) {return shapeRenderer.setShape<MeshShape>();};
	_allocators[SphereShape::identifier] = [](ShapeRenderer& shapeRenderer) -> decltype(auto) {return shapeRenderer.setShape<SphereShape>();};
	_allocators[PlaneShape::identifier] = [](ShapeRenderer& shapeRenderer) -> decltype(auto) {return shapeRenderer.setShape<PlaneShape>();};
}

Shape& ShapeRenderer::setShapeByIdentifier(const std::string& shapeIdentifier)
{
	return _allocators[shapeIdentifier](*this);
}