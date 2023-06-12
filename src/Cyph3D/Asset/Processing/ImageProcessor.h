#pragma once

#include "Cyph3D/Asset/Processing/ImageData.h"

#include <string_view>

class ImageProcessor
{
public:
	ImageData readImageData(std::string_view path, ImageType type, std::string_view cachePath);
};