#pragma once

#include <sigslot/signal.hpp>

class Entity;
class Transform;
class RenderRegistry;
class Camera;
struct ObjectSerialization;

class Component
{
public:
	explicit Component(Entity& entity);
	virtual ~Component() = default;

	Component(const Component& other) = delete;
	Component& operator=(const Component& other) = delete;

	Component(Component&& other) = delete;
	Component& operator=(Component&& other) = delete;
	
	Entity& getEntity() const;
	Transform& getTransform() const;
	
	sigslot::signal<>& getChangedSignal();
	
	virtual void onUpdate();
	virtual void onPreRender(RenderRegistry& renderRegistry, Camera& camera);
	virtual void onDrawUi();
	
	virtual const char* getIdentifier() const = 0;
	
	virtual void duplicate(Entity& targetEntity) const = 0;
	
	virtual ObjectSerialization serialize() const = 0;
	virtual void deserialize(const ObjectSerialization& data) = 0;
	
protected:
	sigslot::signal<> _changed;

private:
	Entity& _entity;
};