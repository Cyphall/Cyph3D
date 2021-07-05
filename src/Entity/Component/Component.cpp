#include "Component.h"
#include "../Entity.h"

Component::Component(Entity& entity):
_entity(entity)
{}

Entity& Component::getEntity() const
{
	return _entity;
}

void Component::onUpdate()
{}

void Component::onPreRender(RenderContext& context)
{}

void Component::onDrawUi()
{}

Transform& Component::getTransform() const
{
	return getEntity().getTransform();
}
