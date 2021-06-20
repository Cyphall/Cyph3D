#pragma once

#include "Resource.h"
#include "../GLObject/Mesh.h"

class Model : public Resource<Mesh>
{
public:
	using Resource::Resource;
	Model(const Model& other) = delete;
	
private:
	void loadResourceImpl() override;
};


