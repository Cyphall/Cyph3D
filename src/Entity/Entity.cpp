#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include "Entity.h"
#include "../Scene/Scene.h"
#include "Component/Animator.h"
#include "Component/DirectionalLight.h"
#include "Component/PointLight.h"
#include "Component/MeshRenderer.h"

std::map<std::string, std::function<Component&(Entity&)>> Entity::_allocators;

Entity::Entity(Transform& parent, Scene& scene):
_transform(this, &parent), _scene(scene)
{

}

ComponentIterator Entity::components_begin()
{
	return ComponentIterator(_components.begin());
}

ComponentIterator Entity::components_end()
{
	return ComponentIterator(_components.end());
}

ComponentConstIterator Entity::components_cbegin() const
{
	return ComponentConstIterator(_components.cbegin());
}

ComponentConstIterator Entity::components_cend() const
{
	return ComponentConstIterator(_components.cend());
}

ComponentIterator Entity::removeComponent(ComponentIterator where)
{
	return ComponentIterator(_components.erase(where.getUnderlyingIterator()));
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
	for (auto it = components_begin(); it != components_end(); it++)
	{
		it->onUpdate();
	}
}

void Entity::onPreRender()
{
	for (auto it = components_begin(); it != components_end(); it++)
	{
		it->onPreRender();
	}
}

EntitySerialization Entity::serialize() const
{
	EntitySerialization data(1);
	
	data.json["name"] = getName();
	
	const Transform& transform = getTransform();
	nlohmann::ordered_json transformData;
	
	glm::vec3 position = transform.getLocalPosition();
	transformData["position"] = {position.x, position.y, position.z};
	glm::quat rotation = transform.getLocalRotation();
	transformData["rotation"] = {rotation.w, rotation.x, rotation.y, rotation.z};
	glm::vec3 scale = transform.getLocalScale();
	transformData["scale"] = {scale.x, scale.y, scale.z};
	
	data.json["transform"] = transformData;
	
	std::vector<nlohmann::ordered_json> components(_components.size());
	
	auto it = components_cbegin();
	for (int i = 0; it != components_cend(); i++, it++)
	{
		ComponentSerialization serialization = it->serialize();
		components[i]["identifier"] = std::string(it->getIdentifier());
		components[i]["version"] = serialization.version;
		components[i]["data"] = serialization.json;
	}
	
	data.json["components"] = components;
	
	return data;
}

void Entity::deserialize(const EntitySerialization& data)
{
	setName(data.json["name"].get<std::string>());
	
	Transform& transform = getTransform();
	transform.setLocalPosition(glm::make_vec3(data.json["transform"]["position"].get<std::vector<float>>().data()));
	std::vector<float> quat = data.json["transform"]["rotation"].get<std::vector<float>>();
	transform.setLocalRotation(glm::quat(quat[0], quat[1], quat[2], quat[3]));
	transform.setLocalScale(glm::make_vec3(data.json["transform"]["scale"].get<std::vector<float>>().data()));
	
	for (const nlohmann::ordered_json& json : data.json["components"])
	{
		Component& component = addComponentByIdentifier(json["identifier"].get<std::string>());
		ComponentSerialization serialization(json["version"].get<int>());
		serialization.json = json["data"];
		component.deserialize(serialization);
	}
}

const std::string& Entity::getName() const
{
	return _name;
}

void Entity::setName(std::string name)
{
	_name = name;
}

void Entity::onDrawUi()
{
	ImGui::PushID(this);
	ImGui::InputText("Name", &_name);
	
	ImGui::Separator();
	
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
	
	for (auto it = components_begin(); it != components_end();)
	{
		ImGui::PushID(&(*it));
		ImGui::Separator();

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
	
	ImGui::PopID();
}

void Entity::initAllocators()
{
	_allocators[MeshRenderer::identifier] = [](Entity& entity) -> decltype(auto) {return entity.addComponent<MeshRenderer>();};
	_allocators[Animator::identifier] = [](Entity& entity) -> decltype(auto) {return entity.addComponent<Animator>();};
	_allocators[PointLight::identifier] = [](Entity& entity) -> decltype(auto) {return entity.addComponent<PointLight>();};
	_allocators[DirectionalLight::identifier] = [](Entity& entity) -> decltype(auto) {return entity.addComponent<DirectionalLight>();};
}

Component& Entity::addComponentByIdentifier(const std::string& identifier)
{
	return _allocators[identifier](*this);
}

std::map<std::string, std::function<Component&(Entity&)>>::iterator Entity::allocators_begin()
{
	return _allocators.begin();
}

std::map<std::string, std::function<Component&(Entity&)>>::iterator Entity::allocators_end()
{
	return _allocators.end();
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
	
	for (auto it = components_cbegin(); it != components_cend(); it++)
	{
		it->duplicate(newEntity);
	}
	
	for (Transform* child : getTransform().getChildren())
	{
		child->getOwner()->duplicate(newEntity.getTransform());
	}
}
