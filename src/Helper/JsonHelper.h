#pragma once

#include <nlohmann/json.hpp>

class JsonHelper
{
public:
	static nlohmann::json loadJsonFromFile(const std::string& path);
	static void saveJsonToFile(const nlohmann::json& json, const std::string& path, bool beautify = false);
};