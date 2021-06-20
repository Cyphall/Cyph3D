#pragma once

#include "Resource.h"
#include "../GLObject/Texture.h"
#include "StbImage.h"
#include "../Enums/ImageType.h"

class Image : public Resource<Texture, ImageType>
{
public:
	using Resource::Resource;
	Image(const Image& other) = delete;
	
private:
	void loadResourceImpl(ImageType type) override;
	
	friend class ResourceManager;
};


