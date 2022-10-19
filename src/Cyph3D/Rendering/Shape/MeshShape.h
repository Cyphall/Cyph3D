#pragma once

#include "Cyph3D/Rendering/Shape/Shape.h"

#include <string>
#include <optional>

class ModelAsset;

class MeshShape : public Shape
{
public:
	explicit MeshShape(ShapeRenderer& shapeRenderer);

	const std::string* getModelPath() const;
	void setModelPath(std::optional<std::string_view> path);
	ModelAsset* getModel() const;
	
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
	std::optional<std::string> _modelPath;
	ModelAsset* _model = nullptr;
};