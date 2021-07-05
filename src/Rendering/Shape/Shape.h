#pragma once

#include "../../ObjectSerialization.h"
#include "../../GLObject/Mesh.h"

class ShapeRenderer;

class Shape
{
public:
	explicit Shape(ShapeRenderer& shapeRenderer);
	virtual ~Shape() = default;
	
	ShapeRenderer& getShapeRenderer() const;
	
	virtual bool isReadyForRasterisationRender() const = 0;
	virtual bool isReadyForRaytracingRender() const = 0;
	
	virtual const Mesh& getMeshToRender() const = 0;
	
	virtual void onDrawUi();
	
	virtual const char* getIdentifier() const = 0;
	
	virtual void duplicate(ShapeRenderer& targetShapeRenderer) const = 0;
	
	virtual ObjectSerialization serialize() const = 0;
	virtual void deserialize(const ObjectSerialization& serialization) = 0;
	
private:
	ShapeRenderer& _shapeRenderer;
};
