#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/Rendering/VertexData.h"

struct ObjectSerialization;
template<typename T>
class VKBuffer;
class ShapeRenderer;
//TODO: remove Shape and go back to MeshRenderer
class Shape
{
public:
	explicit Shape(ShapeRenderer& shapeRenderer);
	virtual ~Shape() = default;
	
	ShapeRenderer& getShapeRenderer() const;
	
	virtual bool isReadyForRasterisationRender() const = 0;
	virtual bool isReadyForRaytracingRender() const = 0;
	
	virtual const VKPtr<VKBuffer<VertexData>>& getVertexBuffer() const = 0;
	virtual const VKPtr<VKBuffer<uint32_t>>& getIndexBuffer() const = 0;
	
	virtual void onDrawUi();
	
	virtual const char* getIdentifier() const = 0;
	
	virtual void duplicate(ShapeRenderer& targetShapeRenderer) const = 0;
	
	virtual ObjectSerialization serialize() const = 0;
	virtual void deserialize(const ObjectSerialization& serialization) = 0;
	
private:
	ShapeRenderer& _shapeRenderer;
};