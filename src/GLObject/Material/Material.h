#pragma once

#include "../../ResourceManagement/Image.h"
#include "MaterialMapType.h"
#include "MaterialMap.h"
#include "MaterialMapDefinition.h"
#include <map>
#include <tuple>

class ResourceManager;

class Material
{
public:
	const std::string& getName() const;
	void setName(std::string name);
	
	static void initialize();
	
	const Texture& getTexture(MaterialMapType mapType);
	
	static Material* getDefault();
	static Material* getMissing();

private:
	std::map<MaterialMapType, MaterialMap> _maps;
	std::string _name;
	
	static Material* _default;
	static Material* _missing;
	
	static std::map<MaterialMapType, MaterialMapDefinition> _mapDefinitions;
	
	explicit Material(std::string name, ResourceManager* resourceManager);
	
	friend class ResourceManager;
};
