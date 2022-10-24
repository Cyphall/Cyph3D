#include "JsonHelper.h"

#include <filesystem>
#include <fstream>

nlohmann::ordered_json JsonHelper::loadJsonFromFile(const std::filesystem::path& path)
{
	nlohmann::ordered_json root;
	
	std::ifstream jsonFile(path);
	jsonFile >> root;
	jsonFile.close();
	
	return root;
}

void JsonHelper::saveJsonToFile(const nlohmann::ordered_json& json, const std::filesystem::path& path, bool beautify)
{
	std::ofstream jsonFile(path);
	if (beautify)
	{
		jsonFile << std::setfill('\t') << std::setw(1);
	}
	jsonFile << json;
	jsonFile.close();
}