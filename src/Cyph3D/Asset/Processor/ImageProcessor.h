#pragma once

#include "Cyph3D/Asset/Processor/ImageData.h"

#include <string_view>

class ImageProcessor
{
private:
	friend class AssetManager;
	
	static ImageData readImageData(std::string_view path, ImageType type, std::string_view cachePath);
};