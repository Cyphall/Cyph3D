#pragma once

#include <nlohmann/json.hpp>

struct ComponentSerialization
{
	nlohmann::ordered_json data;
	int version;
};