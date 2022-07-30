#pragma once

#include <string>
#include "Cyph3D/Enums/ImageType.h"
#include <vector>

struct MaterialMapDefinition
{
	std::string name;
	ImageType type;
	std::vector<uint8_t> defaultData;
};