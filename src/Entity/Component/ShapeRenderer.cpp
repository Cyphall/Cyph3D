#include "ShapeRenderer.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include "../Entity.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "../../Rendering/Renderer.h"
#include "../../Rendering/Shape/MeshShape.h"
#include "../../Rendering/Shape/SphereShape.h"

const char* ShapeRenderer::identifier = "ShapeRenderer";
std::map<std::string, std::function<Shape&(ShapeRenderer&)>> ShapeRenderer::_allocators;

ShapeRenderer::ShapeRenderer(Entity& entity):
Component(entity), _shape(new MeshShape(*this)), _selectedShape(MeshShape::identifier)
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

Shape& ShapeRenderer::getShape() const
{
	return *_shape.get();
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
	
	serialization.data["shape"] = getShape().serialize().toJson();
	
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

void ShapeRenderer::deserialize(const ObjectSerialization& shapeRendererSerialization)
{
	Scene& scene = getEntity().getScene();
	
	if (!shapeRendererSerialization.data["material"].is_null())
	{
		setMaterial(scene.getRM().requestMaterial(shapeRendererSerialization.data["material"].get<std::string>()));
	}
	
	if (shapeRendererSerialization.identifier == "MeshRenderer")
	{
		MeshShape& shape = setShape<MeshShape>();
		shape.setModel(scene.getRM().requestModel(shapeRendererSerialization.data["model"].get<std::string>()));
	}
	else
	{
		ObjectSerialization shapeSerialization = ObjectSerialization::fromJson(shapeRendererSerialization.data["shape"]);
		Shape& shape = setShapeByIdentifier(shapeSerialization.identifier);
		shape.deserialize(shapeSerialization);
	}
	
	_selectedShape = getShape().getIdentifier();
	
	setContributeShadows(shapeRendererSerialization.data["contribute_shadows"].get<bool>());
}

Material* ShapeRenderer::getDrawingMaterial()
{
	return _material != nullptr ? _material : Material::getMissing();
}

void ShapeRenderer::onPreRender(RenderContext& context)
{
	RenderData data;
	data.material = getDrawingMaterial();
	data.shape = &getShape();
	data.owner = &getEntity();
	data.contributeShadows = getContributeShadows();
	data.matrix = getTransform().getLocalToWorldMatrix();
	
	context.renderer.requestShapeRendering(data);
}

void ShapeRenderer::onDrawUi()
{
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
	newComponent.setMaterial(getMaterial());
	getShape().duplicate(newComponent);
	newComponent.setContributeShadows(getContributeShadows());
}

void ShapeRenderer::initAllocators()
{
	_allocators[MeshShape::identifier] = [](ShapeRenderer& shapeRenderer) -> decltype(auto) {return shapeRenderer.setShape<MeshShape>();};
	_allocators[SphereShape::identifier] = [](ShapeRenderer& shapeRenderer) -> decltype(auto) {return shapeRenderer.setShape<SphereShape>();};
}

Shape& ShapeRenderer::setShapeByIdentifier(const std::string& shapeIdentifier)
{
	return _allocators[shapeIdentifier](*this);
}
