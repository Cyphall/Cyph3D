#pragma once

#include <memory>
#include "Cyph3D/GLObject/Texture.h"
#include "Cyph3D/ResourceManagement/Image.h"

struct MaterialMap
{
	std::unique_ptr<Texture> defaultTexture;
	Image* imageTexture = nullptr;
};