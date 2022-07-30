#pragma once

#include "Cyph3D/Rendering/Shape/Shape.h"

class PlaneShape : public Shape
{
public:
	PlaneShape(ShapeRenderer& shapeRenderer);
	
	bool isReadyForRasterisationRender() const override;
	bool isReadyForRaytracingRender() const override;
	
	const Mesh& getMeshToRender() const override;
	
	void onDrawUi() override;
	
	bool isInfinite() const;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	void duplicate(ShapeRenderer& targetShapeRenderer) const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;

private:
	bool _infinite = false;
};