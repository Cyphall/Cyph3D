#pragma once

#include "Cyph3D/Rendering/Shape/Shape.h"

class SphereShape : public Shape
{
public:
	explicit SphereShape(ShapeRenderer& shapeRenderer);
	
	bool isReadyForRasterisationRender() const override;
	bool isReadyForRaytracingRender() const override;
	
	const VKPtr<VKBuffer<VertexData>>& getVertexBuffer() const override;
	const VKPtr<VKBuffer<uint32_t>>& getIndexBuffer() const override;
	
	void onDrawUi() override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	void duplicate(ShapeRenderer& targetShapeRenderer) const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;
};