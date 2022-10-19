#pragma once

#include "Cyph3D/Asset/Processor/ImageData.h"

#include <crossguid/guid.hpp>

class ImageProcessor
{
private:
	friend class AssetManager;
	
	static ImageData readImageData(const xg::Guid& guid, std::string_view path, const GLenum& format);
};