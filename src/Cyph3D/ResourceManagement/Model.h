#pragma once

#include "Cyph3D/GLObject/Mesh.h"
#include "Cyph3D/ResourceManagement/Resource.h"

template<typename T>
class GLImmutableBuffer;
class ResourceManager;

class Model : public Resource<Mesh>
{
public:
	Model(const std::string& name, ResourceManager& rm);
	
private:
	struct LoadData;
	
	std::unique_ptr<LoadData> _loadData;
	
	void load_step1_tp();
	bool load_step2_mt();
	bool load_step3_mt();
};