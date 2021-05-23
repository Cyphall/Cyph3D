#pragma once

#include <nlohmann/json.hpp>

struct EntitySerialization
{
	nlohmann::ordered_json json;
	int version;
	
	EntitySerialization(int version):
	version(version)
	{}
};
