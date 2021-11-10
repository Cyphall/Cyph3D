#pragma once

#include <memory>
#include "../Texture.h"
#include "../../ResourceManagement/Image.h"

struct MaterialMap
{
	std::unique_ptr<Texture> defaultTexture;
	Image* imageTexture = nullptr;
};