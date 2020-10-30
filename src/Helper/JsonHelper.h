#pragma once

#include <nlohmann/json.hpp>

nlohmann::json loadJsonFromFile(const std::string& path);
void saveJsonToFile(const nlohmann::json& json, const std::string& path, bool beautify = false);