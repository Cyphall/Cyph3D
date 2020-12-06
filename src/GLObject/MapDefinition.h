#pragma once

#include <vector>
#include "../Enums/ImageType.h"

struct MapDefinition
{
	ImageType type;
	std::vector<uint8_t> defaultData;
};
