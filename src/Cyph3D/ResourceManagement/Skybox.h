#pragma once

#include "Cyph3D/GLObject/GLCubemap.h"
#include "Cyph3D/ResourceManagement/Resource.h"

class ResourceManager;

class Skybox : public Resource<GLCubemap>
{
public:
	~Skybox() override;

	float getRotation() const;
	void setRotation(float rotation);

private:
	friend class ResourceManager;

	Skybox(const std::string& name, ResourceManager& rm);

	bool load_step1_mt();
	bool load_step2_mt();
	
	struct LoadData;
	std::unique_ptr<LoadData> _loadData;
	
	float _rotation = 0;
};