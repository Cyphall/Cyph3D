#pragma once

#include <vector>

struct MapDefinition
{
	bool compressed;
	bool sRGB;
	std::vector<uint8_t> defaultData;
};
