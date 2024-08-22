#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>

class JsonHelper
{
public:
	static nlohmann::ordered_json loadJsonFromFile(const std::filesystem::path& path);
	static void saveJsonToFile(const nlohmann::ordered_json& json, const std::filesystem::path& path, bool beautify = false);
};