#pragma once

#include "MapDefinition.h"
#include "ShaderProgram.h"
class ResourceManager;

class MaterialShaderProgram
{
public:
	ShaderProgram* getShaderProgram() const;
	std::map<std::string, MapDefinition>& getMapDefinitions();
private:
	ShaderProgram* _shaderProgram;
	std::map<std::string, MapDefinition> _mapDefinitions;
	
	MaterialShaderProgram(const std::string& layoutName, ResourceManager* resourceManager);
	
	friend class ResourceManager;
};
