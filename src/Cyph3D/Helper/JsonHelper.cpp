#include "JsonHelper.h"
#include <fstream>
#include <filesystem>

nlohmann::ordered_json JsonHelper::loadJsonFromFile(const std::string& path)
{
	nlohmann::ordered_json root;
	
	std::ifstream jsonFile(path);
	jsonFile >> root;
	jsonFile.close();
	
	return root;
}

void JsonHelper::saveJsonToFile(const nlohmann::ordered_json& json, const std::string& path, bool beautify)
{
	std::ofstream jsonFile(path);
	if (beautify)
	{
		jsonFile << std::setw(4);
	}
	jsonFile << json;
	jsonFile.close();
}