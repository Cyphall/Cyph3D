#pragma once

#include <nlohmann/json.hpp>

class JsonHelper
{
public:
	static nlohmann::ordered_json loadJsonFromFile(const std::string& path);
	static void saveJsonToFile(const nlohmann::ordered_json& json, const std::string& path, bool beautify = false);
};