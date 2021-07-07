#pragma once

#include "Shape.h"

class SphereShape : public Shape
{
public:
	SphereShape(ShapeRenderer& shapeRenderer);
	
	bool isReadyForRasterisationRender() const override;
	bool isReadyForRaytracingRender() const override;
	
	const Mesh& getMeshToRender() const override;
	
	void onDrawUi() override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	void duplicate(ShapeRenderer& targetShapeRenderer) const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;
};
