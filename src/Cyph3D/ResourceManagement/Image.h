#pragma once

#include "Cyph3D/Enums/ImageType.h"
#include "Cyph3D/GLObject/Texture.h"
#include "Cyph3D/ResourceManagement/Resource.h"

class Image : public Resource<Texture, ImageType>
{
public:
	using Resource::Resource;
	Image(const Image& other) = delete;
	
private:
	void loadResourceImpl(ImageType type) override;
	
	friend class ResourceManager;
};