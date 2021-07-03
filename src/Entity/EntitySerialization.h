#pragma once

#include <nlohmann/json.hpp>

struct EntitySerialization
{
	nlohmann::ordered_json data;
	int version;
};
