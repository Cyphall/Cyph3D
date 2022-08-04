#pragma once

#include "Cyph3D/GLObject/GLCubemap.h"
#include "Cyph3D/ResourceManagement/Resource.h"

class ResourceManager;

class Skybox : public Resource<GLCubemap>
{
public:
	Skybox(const std::string& name, ResourceManager& rm);
	~Skybox();

	float getRotation() const;
	void setRotation(float rotation);

private:
	struct LoadData;
	std::unique_ptr<LoadData> _loadData;
	
	float _rotation = 0;

	bool load_step1_mt();
	bool load_step2_mt();
};