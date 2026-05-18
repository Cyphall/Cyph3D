#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>

namespace c3d
{
class JsonHelper
{
public:
	static nlohmann::ordered_json loadJsonFromFile(const std::filesystem::path& path);
	static void saveJsonToFile(const nlohmann::ordered_json& json, const std::filesystem::path& path, bool beautify = false);
};
}