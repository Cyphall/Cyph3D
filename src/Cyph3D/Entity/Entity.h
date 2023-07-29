#pragma once

#include "Cyph3D/Scene/Transform.h"
#include "Cyph3D/Entity/Component/Component.h"
#include "Cyph3D/UI/IInspectable.h"

#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <vector>
#include <sigslot/signal.hpp>

class Scene;
class Component;
class ComponentIterator;
class ComponentConstIterator;
class RenderRegistry;
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
	
	ComponentIterator begin();
	ComponentIterator end();
	ComponentConstIterator begin() const;
	ComponentConstIterator end() const;
	
	template<typename T>
	T& addComponent()
	{
		ComponentContainer& container = _components.emplace_back();
		container.component = std::make_unique<T>(*this);
		container.componentChangedConnection = container.component->getChangedSignal().connect([this](){
			_changed();
		});
		
		_changed();
		
		return *static_cast<T*>(container.component.get());
	}
	
	ComponentIterator removeComponent(ComponentIterator where);
	
	Transform& getTransform();
	const Transform& getTransform() const;
	
	const std::string& getName() const;
	void setName(const std::string& name);
	
	void onDrawUi() override;
	void onUpdate();
	void onPreRender(RenderRegistry& renderRegistry, Camera& camera);
	
	Scene& getScene() const;
	
	void duplicate(Transform& parent) const;
	
	ObjectSerialization serialize() const;
	void deserialize(const ObjectSerialization& entitySerialization);
	
	sigslot::signal<>& getChangedSignal();
	
	static std::map<std::string, std::function<Component&(Entity&)>>::iterator componentFactories_begin();
	static std::map<std::string, std::function<Component&(Entity&)>>::iterator componentFactories_end();

private:
	struct ComponentContainer
	{
		std::unique_ptr<Component> component;
		sigslot::connection componentChangedConnection;
	};
	
	std::string _name = "New Entity";
	std::vector<ComponentContainer> _components;
	Scene& _scene;
	Transform _transform;
	sigslot::connection _transformChangedConnection;
	
	sigslot::signal<> _changed;
	
	Component& addComponentByIdentifier(const std::string& identifier);
	
	static std::map<std::string, std::function<Component&(Entity&)>> _componentFactories;
	static void initComponentFactories();
	
	friend class Engine;
	friend class ComponentIterator;
	friend class ComponentConstIterator;
};