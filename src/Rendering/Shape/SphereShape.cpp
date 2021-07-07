#include "SphereShape.h"
#include "../../Entity/Component/ShapeRenderer.h"

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

const Mesh& SphereShape::getMeshToRender() const
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
