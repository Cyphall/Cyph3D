#pragma once

#include "Cyph3D/Enums/ImageType.h"

#include <string>
#include <vector>

struct MaterialMapDefinition
{
	std::string name;
	ImageType type;
	std::vector<uint8_t> defaultData;
};