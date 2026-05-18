#include "Component.h"

#include "Cyph3D/Entity/Entity.h"

c3d::Component::Component(Entity& entity):
	_entity(entity)
{}

c3d::Entity& c3d::Component::getEntity() const
{
	return _entity;
}

void c3d::Component::onUpdate()
{}

void c3d::Component::onPreRender(RenderRegistry& renderRegistry, Camera& camera)
{}

void c3d::Component::onDrawUi()
{}

c3d::Transform& c3d::Component::getTransform() const
{
	return getEntity().getTransform();
}

sigslot::signal<>& c3d::Component::getChangedSignal()
{
	return _changed;
}