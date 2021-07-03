#pragma once

#include "Component/Component.h"
#include "../Scene/Transform.h"
#include "EntitySerialization.h"
#include "../Iterator/ComponentIterator.h"
#include "../Iterator/ComponentConstIterator.h"
#include <memory>
#include <vector>
#include <string>

#include <nlohmann/json.hpp>

class Scene;
class Renderer;

class Entity
{
public:
	Entity(Transform& parent, Scene& scene);
	Entity(const Entity& other) = delete;
	Entity(Entity&&) = delete;
	
	ComponentIterator components_begin();
	ComponentIterator components_end();
	ComponentConstIterator components_cbegin() const;
	ComponentConstIterator components_cend() const;
	
	template<typename T>
	T& addComponent();
	ComponentIterator removeComponent(ComponentIterator where);
	
	Transform& getTransform();
	const Transform& getTransform() const;
	
	const std::string& getName() const;
	void setName(std::string name);
	
	void onDrawUi();
	void onUpdate();
	void onPreRender(RenderContext& context);
	
	Scene& getScene() const;
	
	void duplicate(Transform& parent) const;
	
	EntitySerialization serialize() const;
	void deserialize(const EntitySerialization& entitySerialization);
	
	static std::map<std::string, std::function<Component&(Entity&)>>::iterator allocators_begin();
	static std::map<std::string, std::function<Component&(Entity&)>>::iterator allocators_end();

private:
	std::string _name = "New Entity";
	std::vector<std::unique_ptr<Component>> _components;
	Scene& _scene;
	Transform _transform;
	
	Component& addComponentByIdentifier(const std::string& identifier);
	
	static std::map<std::string, std::function<Component&(Entity&)>> _allocators;
	static void initAllocators();
	
	friend class Engine;
};

template<typename T>
T& Entity::addComponent()
{
	T* component = new T(*this);
	_components.emplace_back(component);
	return *component;
}