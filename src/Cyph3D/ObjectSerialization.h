#pragma once

#include <nlohmann/json.hpp>

struct ObjectSerialization
{
	nlohmann::ordered_json data;
	int version;
	std::string identifier;

	nlohmann::ordered_json toJson();
	static ObjectSerialization fromJson(const nlohmann::ordered_json& json);
};