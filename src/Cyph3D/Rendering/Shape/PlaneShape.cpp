#include "Cyph3D/Rendering/Shape/PlaneShape.h"
#include <imgui.h>
#include "Cyph3D/Entity/Component/ShapeRenderer.h"
#include "Cyph3D/ObjectSerialization.h"
#include <stdexcept>

const char* PlaneShape::identifier = "PlaneShape";

PlaneShape::PlaneShape(ShapeRenderer& shapeRenderer):
Shape(shapeRenderer)
{
	
}

bool PlaneShape::isReadyForRasterisationRender() const
{
	return false;
}

bool PlaneShape::isReadyForRaytracingRender() const
{
	return true;
}

const Mesh& PlaneShape::getMeshToRender() const
{
	throw std::runtime_error("This function is not implemented");
}

void PlaneShape::onDrawUi()
{
	ImGui::Checkbox("Infinite", &_infinite);
}

const char* PlaneShape::getIdentifier() const
{
	return identifier;
}

void PlaneShape::duplicate(ShapeRenderer& targetShapeRenderer) const
{
	PlaneShape& newShape = targetShapeRenderer.setShape<PlaneShape>();
	newShape._infinite = _infinite;
}

ObjectSerialization PlaneShape::serialize() const
{
	ObjectSerialization serialization;
	serialization.version = 1;
	serialization.identifier = getIdentifier();
	
	serialization.data["infinite"] = _infinite;
	
	return serialization;
}

void PlaneShape::deserialize(const ObjectSerialization& serialization)
{
	_infinite = serialization.data["infinite"].get<bool>();
}

bool PlaneShape::isInfinite() const
{
	return _infinite;
}