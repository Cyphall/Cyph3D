#include "Cyph3D/Rendering/Shape/Shape.h"

Shape::Shape(ShapeRenderer& shapeRenderer):
_shapeRenderer(shapeRenderer)
{

}

void Shape::onDrawUi()
{}

ShapeRenderer& Shape::getShapeRenderer() const
{
	return _shapeRenderer;
}