#pragma once

#include "Resource.h"
#include "../GLObject/Mesh.h"

struct ModelLoadingData
{
	std::vector<Mesh::VertexData> vertexData;
	std::vector<GLuint> indices;
};

class Model : public Resource<Mesh, ModelLoadingData>
{
public:
	using Resource::Resource;
	Model(const Model& other) = delete;
	
private:
	void finishLoading(const ModelLoadingData& data) override;
	
	static ModelLoadingData loadFromFile(const std::string& name);
	
	friend class ResourceManager;
};


