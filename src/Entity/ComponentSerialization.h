#pragma once

#include <nlohmann/json.hpp>

struct ComponentSerialization
{
	nlohmann::ordered_json json;
	int version;
	
	ComponentSerialization(int version):
	version(version)
	{}
};