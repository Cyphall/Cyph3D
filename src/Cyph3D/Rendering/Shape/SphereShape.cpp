#include "SphereShape.h"

#include "Cyph3D/Entity/Component/ShapeRenderer.h"
#include "Cyph3D/ObjectSerialization.h"

#include <stdexcept>

const char* SphereShape::identifier = "SphereShape";

SphereShape::SphereShape(ShapeRenderer& shapeRenderer):
Shape(shapeRenderer)
{

}

bool SphereShape::isReadyForRasterisationRender() const
{
	return false;
}

bool SphereShape::isReadyForRaytracingRender() const
{
	return true;
}

const VKPtr<VKBuffer<VertexData>>& SphereShape::getVertexBuffer() const
{
	throw std::runtime_error("This function is not implemented");
}

const VKPtr<VKBuffer<uint32_t>>& SphereShape::getIndexBuffer() const
{
	throw std::runtime_error("This function is not implemented");
}

void SphereShape::onDrawUi()
{

}

const char* SphereShape::getIdentifier() const
{
	return identifier;
}

void SphereShape::duplicate(ShapeRenderer& targetShapeRenderer) const
{
	targetShapeRenderer.setShape<SphereShape>();
}

ObjectSerialization SphereShape::serialize() const
{
	ObjectSerialization serialization;
	serialization.version = 1;
	serialization.identifier = getIdentifier();
	return serialization;
}

void SphereShape::deserialize(const ObjectSerialization& serialization)
{

}