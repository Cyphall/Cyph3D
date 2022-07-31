#pragma once

#include <memory>

class Texture;
class Image;

struct MaterialMap
{
	std::unique_ptr<Texture> defaultTexture;
	Image* imageTexture = nullptr;
};