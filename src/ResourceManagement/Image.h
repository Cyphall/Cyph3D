#pragma once

#include "Resource.h"
#include "../GLObject/Texture.h"
#include "StbImage.h"

struct ImageLoadingData
{
	GLenum internalFormat;
	GLenum pixelFormat;
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
	
	static ImageLoadingData loadFromFile(const std::string& name, bool sRGB, bool compressed);
	
	friend class ResourceManager;
};


