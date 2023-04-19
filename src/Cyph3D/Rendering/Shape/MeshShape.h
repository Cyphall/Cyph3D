#pragma once

#include "Cyph3D/Rendering/Shape/Shape.h"

#include <string>
#include <optional>

class MeshAsset;

class MeshShape : public Shape
{
public:
	explicit MeshShape(ShapeRenderer& shapeRenderer);

	const std::string* getMeshPath() const;
	void setMeshPath(std::optional<std::string_view> path);
	MeshAsset* getMesh() const;
	
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

private:
	std::optional<std::string> _meshPath;
	MeshAsset* _mesh = nullptr;
};