#pragma once

#include "Cyph3D/Rendering/Shape/Shape.h"
#include "Cyph3D/ResourceManagement/Model.h"

class MeshShape : public Shape
{
public:
	MeshShape(ShapeRenderer& shapeRenderer);
	
	const Model* getModel();
	const Model* getModel() const;
	void setModel(const Model* model);
	
	bool isReadyForRasterisationRender() const override;
	bool isReadyForRaytracingRender() const override;
	
	const Mesh& getMeshToRender() const override;
	
	void onDrawUi() override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	void duplicate(ShapeRenderer& targetShapeRenderer) const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;

private:
	const Model* _model = nullptr;
};