#pragma once

#include "Cyph3D/Scene/Transform.h"
#include "Cyph3D/UI/IInspectable.h"

#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <vector>

class Scene;
class Component;
class ComponentIterator;
class ComponentConstIterator;
class SceneRenderer;
class Camera;
struct ObjectSerialization;

class Entity : public IInspectable
{
public:
	Entity(Transform& parent, Scene& scene);

	Entity(const Entity& other) = delete;
	Entity& operator=(const Entity& other) = delete;

	Entity(Entity&& other) = delete;
	Entity& operator=(Entity&& other) = delete;
	
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
	
	void onDrawUi() override;
	void onUpdate();
	void onPreRender(SceneRenderer& sceneRenderer, Camera& camera);
	
	Scene& getScene() const;
	
	void duplicate(Transform& parent) const;
	
	ObjectSerialization serialize() const;
	void deserialize(const ObjectSerialization& entitySerialization);
	
	static std::map<std::string, std::function<Component&(Entity&)>>::iterator componentFactories_begin();
	static std::map<std::string, std::function<Component&(Entity&)>>::iterator componentFactories_end();

private:
	std::string _name = "New Entity";
	std::vector<std::unique_ptr<Component>> _components;
	Scene& _scene;
	Transform _transform;
	
	Component& addComponentByIdentifier(const std::string& identifier);
	
	static std::map<std::string, std::function<Component&(Entity&)>> _componentFactories;
	static void initComponentFactories();
	
	friend class Engine;
};

template<typename T>
T& Entity::addComponent()
{
	T* component = new T(*this);
	_components.emplace_back(component);
	return *component;
}