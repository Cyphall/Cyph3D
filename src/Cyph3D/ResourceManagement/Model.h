#pragma once

#include "Cyph3D/GLObject/Mesh.h"
#include "Cyph3D/ResourceManagement/Resource.h"

class Model : public Resource<Mesh>
{
public:
	using Resource::Resource;
	Model(const Model& other) = delete;
	
private:
	void loadResourceImpl() override;
};

