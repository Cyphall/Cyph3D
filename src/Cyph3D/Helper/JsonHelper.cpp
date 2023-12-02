#include "JsonHelper.h"

#include "Cyph3D/Helper/FileHelper.h"

#include <filesystem>
#include <fstream>

nlohmann::ordered_json JsonHelper::loadJsonFromFile(const std::filesystem::path& path)
{
	std::ifstream jsonFile = FileHelper::openFileForReading(path);

	nlohmann::ordered_json root;
	jsonFile >> root;
	jsonFile.close();

	return root;
}

void JsonHelper::saveJsonToFile(const nlohmann::ordered_json& json, const std::filesystem::path& path, bool beautify)
{
	std::ofstream jsonFile = FileHelper::openFileForWriting(path);

	if (beautify)
	{
		jsonFile << std::setfill('\t') << std::setw(1);
	}
	jsonFile << json;
	jsonFile.close();
}