#pragma once

#include "../../Scene/Transform.h"
#include "../../RenderContext.h"
#include "../../ObjectSerialization.h"

class Entity;
class Renderer;

class Component
{
public:
	explicit Component(Entity& entity);
	virtual ~Component() = default;
	
	Entity& getEntity() const;
	Transform& getTransform() const;
	
	virtual void onUpdate();
	virtual void onPreRender(RenderContext& context);
	virtual void onDrawUi();
	
	virtual const char* getIdentifier() const = 0;
	
	virtual void duplicate(Entity& targetEntity) const = 0;
	
	virtual ObjectSerialization serialize() const = 0;
	virtual void deserialize(const ObjectSerialization& data) = 0;

private:
	Entity* _entity;
};