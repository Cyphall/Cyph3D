#pragma once

#include "Resource.h"
#include "../GLObject/Texture.h"
#include "StbImage.h"
#include "../Enums/ImageType.h"

struct ImageLoadingData
{
	GLenum internalFormat;
	StbImage data;
	std::array<GLint, 4> swizzle;
};

class Image : public Resource<Texture, ImageLoadingData>
{
public:
	using Resource::Resource;
	Image(const Image& other) = delete;
private:
	void finishLoading(const ImageLoadingData& data) override;
	
	static ImageLoadingData loadFromFile(const std::string& name, ImageType type);
	
	friend class ResourceManager;
};


