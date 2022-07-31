#pragma once

#include <memory>

class GLTexture;
class Image;

struct MaterialMap
{
	std::unique_ptr<GLTexture> defaultTexture;
	Image* imageTexture = nullptr;
};