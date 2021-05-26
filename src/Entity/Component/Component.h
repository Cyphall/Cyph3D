#pragma once

#include "../ComponentSerialization.h"
#include "../../Scene/Transform.h"
#include "../../RenderContext.h"

class Entity;
class Renderer;

class Component
{
public:
	Component(Entity& entity);
	
	Entity& getEntity() const;
	Transform& getTransform() const;
	
	virtual void onUpdate();
	virtual void onPreRender(RenderContext& context);
	virtual void onDrawUi();
	
	virtual const char* getIdentifier() const = 0;
	
	virtual void duplicate(Entity& targetEntity) const = 0;
	
	virtual ComponentSerialization serialize() const = 0;
	virtual void deserialize(const ComponentSerialization& data) = 0;

private:
	Entity* _entity;
};