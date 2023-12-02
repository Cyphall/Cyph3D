#include "Entity.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Component/Animator.h"
#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/Entity/Component/ModelRenderer.h"
#include "Cyph3D/Entity/Component/PointLight.h"
#include "Cyph3D/Iterator/ComponentConstIterator.h"
#include "Cyph3D/Iterator/ComponentIterator.h"
#include "Cyph3D/ObjectSerialization.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Window.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

std::map<std::string, std::function<Component&(Entity&)>> Entity::_componentFactories;

Entity::Entity(Transform& parent, Scene& scene):
	_transform(this, &parent),
	_scene(scene)
{
	_transformChangedConnection = _transform.getChangedSignal().connect(
		[this]()
		{
			_changed();
		}
	);
}

ComponentIterator Entity::begin()
{
	return ComponentIterator(_components.begin());
}

ComponentIterator Entity::end()
{
	return ComponentIterator(_components.end());
}

ComponentConstIterator Entity::begin() const
{
	return ComponentConstIterator(_components.cbegin());
}

ComponentConstIterator Entity::end() const
{
	return ComponentConstIterator(_components.cend());
}

ComponentIterator Entity::removeComponent(ComponentIterator where)
{
	ComponentIterator newIt = ComponentIterator(_components.erase(where.getUnderlyingIterator()));

	_changed();

	return newIt;
}

Transform& Entity::getTransform()
{
	return _transform;
}

const Transform& Entity::getTransform() const
{
	return _transform;
}

Scene& Entity::getScene() const
{
	return _scene;
}

void Entity::onUpdate()
{
	for (Component& component : *this)
	{
		component.onUpdate();
	}
}

void Entity::onPreRender(RenderRegistry& renderRegistry, Camera& camera)
{
	for (Component& component : *this)
	{
		component.onPreRender(renderRegistry, camera);
	}
}

ObjectSerialization Entity::serialize() const
{
	ObjectSerialization entitySerialization;

	entitySerialization.version = 1;
	entitySerialization.identifier = "Entity";

	entitySerialization.data["name"] = getName();

	const Transform& transform = getTransform();
	nlohmann::ordered_json transformData;

	glm::vec3 position = transform.getLocalPosition();
	transformData["position"] = {position.x, position.y, position.z};
	glm::quat rotation = transform.getLocalRotation();
	transformData["rotation"] = {rotation.w, rotation.x, rotation.y, rotation.z};
	glm::vec3 scale = transform.getLocalScale();
	transformData["scale"] = {scale.x, scale.y, scale.z};

	entitySerialization.data["transform"] = transformData;

	std::vector<nlohmann::ordered_json> jsonComponents;
	jsonComponents.reserve(_components.size());
	for (const Component& component : *this)
	{
		jsonComponents.emplace_back(component.serialize().toJson());
	}

	entitySerialization.data["components"] = jsonComponents;

	return entitySerialization;
}

void Entity::deserialize(const ObjectSerialization& entitySerialization)
{
	setName(entitySerialization.data["name"].get<std::string>());

	Transform& transform = getTransform();
	transform.setLocalPosition(glm::make_vec3(entitySerialization.data["transform"]["position"].get<std::vector<float>>().data()));
	std::vector<float> quat = entitySerialization.data["transform"]["rotation"].get<std::vector<float>>();
	transform.setLocalRotation(glm::quat(quat[0], quat[1], quat[2], quat[3]));
	transform.setLocalScale(glm::make_vec3(entitySerialization.data["transform"]["scale"].get<std::vector<float>>().data()));

	for (const nlohmann::ordered_json& json : entitySerialization.data["components"])
	{
		ObjectSerialization componentSerialization = ObjectSerialization::fromJson(json);

		Component& component = addComponentByIdentifier(componentSerialization.identifier);
		component.deserialize(componentSerialization);
	}
}

sigslot::signal<>& Entity::getChangedSignal()
{
	return _changed;
}

const std::string& Entity::getName() const
{
	return _name;
}

void Entity::setName(const std::string& name)
{
	_name = name;

	_changed();
}

void Entity::onDrawUi()
{
	ImGui::PushID(this);
	ImGui::InputText("Name", &_name);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		glm::vec3 position = _transform.getLocalPosition();
		if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f))
		{
			_transform.setLocalPosition(position);
		}

		glm::vec3 rotation = _transform.getEulerLocalRotation();
		if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.01f))
		{
			_transform.setEulerLocalRotation(rotation);
		}

		glm::vec3 scale = _transform.getLocalScale();
		if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f))
		{
			_transform.setLocalScale(scale);
		}
	}

	for (auto it = begin(); it != end();)
	{
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::PushID(&(*it));

		bool keep = true;
		if (ImGui::CollapsingHeader(it->getIdentifier(), &keep, ImGuiTreeNodeFlags_DefaultOpen))
		{
			it->onDrawUi();
		}

		ImGui::PopID();

		if (keep)
		{
			it++;
		}
		else
		{
			it = removeComponent(it);
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	const char* addComponentText = "Add Component";

	float availableWidth = ImGui::GetContentRegionAvail().x;
	float wantedButtonWidth = ImGui::CalcTextSize(addComponentText).x + ImGui::GetStyle().FramePadding.x * 2 + 50.0f * Engine::getWindow().getPixelScale();
	float actualButtonWidth = std::min(wantedButtonWidth, availableWidth);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((availableWidth - actualButtonWidth) / 2));

	if (ImGui::Button(addComponentText, ImVec2(actualButtonWidth, 0)))
		ImGui::OpenPopup("add_component");

	if (ImGui::BeginPopup("add_component"))
	{
		for (auto it = Entity::componentFactories_begin(); it != Entity::componentFactories_end(); it++)
		{
			if (ImGui::Selectable(it->first.c_str()))
			{
				it->second(*this);
			}
		}
		ImGui::EndPopup();
	}

	ImGui::PopID();
}

void Entity::initComponentFactories()
{
	_componentFactories[ModelRenderer::identifier] = [](Entity& entity) -> decltype(auto)
	{
		return entity.addComponent<ModelRenderer>();
	};

	// for backwards compatibility
	_componentFactories["ShapeRenderer"] = [](Entity& entity) -> decltype(auto)
	{
		return entity.addComponent<ModelRenderer>();
	};

	_componentFactories[Animator::identifier] = [](Entity& entity) -> decltype(auto)
	{
		return entity.addComponent<Animator>();
	};

	_componentFactories[PointLight::identifier] = [](Entity& entity) -> decltype(auto)
	{
		return entity.addComponent<PointLight>();
	};

	_componentFactories[DirectionalLight::identifier] = [](Entity& entity) -> decltype(auto)
	{
		return entity.addComponent<DirectionalLight>();
	};
}

Component& Entity::addComponentByIdentifier(const std::string& identifier)
{
	return _componentFactories[identifier](*this);
}

std::map<std::string, std::function<Component&(Entity&)>>::iterator Entity::componentFactories_begin()
{
	return _componentFactories.begin();
}

std::map<std::string, std::function<Component&(Entity&)>>::iterator Entity::componentFactories_end()
{
	return _componentFactories.end();
}

void Entity::duplicate(Transform& parent) const
{
	Entity& newEntity = getScene().createEntity(parent);
	newEntity.setName(getName());

	const Transform& thisTransform = getTransform();
	Transform& newTransform = newEntity.getTransform();
	newTransform.setLocalPosition(thisTransform.getLocalPosition());
	newTransform.setLocalRotation(thisTransform.getLocalRotation());
	newTransform.setLocalScale(thisTransform.getLocalScale());

	for (const Component& component : *this)
	{
		component.duplicate(newEntity);
	}

	for (Transform* child : getTransform().getChildren())
	{
		child->getOwner()->duplicate(newEntity.getTransform());
	}
}