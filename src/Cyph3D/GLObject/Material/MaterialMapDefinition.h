#pragma once

#include <string>
#include "../../Enums/ImageType.h"
#include <vector>

struct MaterialMapDefinition
{
	std::string name;
	ImageType type;
	std::vector<uint8_t> defaultData;
};
