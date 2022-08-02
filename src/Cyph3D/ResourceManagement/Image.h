#pragma once

#include "Cyph3D/Enums/ImageType.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/ResourceManagement/Resource.h"

class Image : public Resource<GLTexture, ImageType>
{
public:
	Image(const std::string& name, ImageType type, ResourceManager& rm);
	~Image();
	
private:
	struct LoadData;
	std::unique_ptr<LoadData> _loadData;
	
	void load_step1_tp();
	bool load_step2_mt();
	bool load_step3_mt();
};