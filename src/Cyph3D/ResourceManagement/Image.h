#pragma once

#include "Cyph3D/Enums/ImageType.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/ResourceManagement/Resource.h"

class ResourceManager;

class Image : public Resource<GLTexture>
{
public:
	Image(const std::string& name, ImageType type, ResourceManager& rm);
	~Image();
	
private:
	struct LoadData;
	std::unique_ptr<LoadData> _loadData;
	
	bool load_step1_mt();
	bool load_step2_mt();
};