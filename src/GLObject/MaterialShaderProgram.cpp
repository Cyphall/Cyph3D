#include "MaterialShaderProgram.h"
#include "../ResourceManagement/ResourceManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <fmt/core.h>

MaterialShaderProgram::MaterialShaderProgram(const std::string& layoutName, ResourceManager* resourceManager)
{
	ShaderProgramCreateInfo createInfo;
	createInfo.shadersFiles[GL_VERTEX_SHADER].emplace_back("internal/gbuffer/renderToGbuffer");
	createInfo.shadersFiles[GL_FRAGMENT_SHADER].emplace_back(fmt::format("materialLayout/{}", layoutName));
	
	_shaderProgram = resourceManager->requestShaderProgram(createInfo);
	
	nlohmann::json root;
	
	std::ifstream jsonFile(fmt::format("resources/shaders/materialLayout/{}.json", layoutName));
	jsonFile >> root;
	jsonFile.close();
	
	for (const auto& [name, definition] : root.items())
	{
		MapDefinition mapDefinition;
		
		nlohmann::json defaultDataJson = definition["default_data"];
		mapDefinition.defaultData.reserve(defaultDataJson.size());
		mapDefinition.defaultData.insert(mapDefinition.defaultData.end(), defaultDataJson.begin(), defaultDataJson.end());
		
		mapDefinition.compressed = definition["compressed"];
		mapDefinition.sRGB = definition["srgb"];
		
		_mapDefinitions[name] = std::move(mapDefinition);
	}
}

ShaderProgram* MaterialShaderProgram::getShaderProgram() const
{
	return _shaderProgram;
}

std::map<std::string, MapDefinition>& MaterialShaderProgram::getMapDefinitions()
{
	return _mapDefinitions;
}
