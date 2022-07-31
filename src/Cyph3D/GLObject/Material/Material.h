#pragma once

#include "Cyph3D/GLObject/Material/MaterialMap.h"
#include "Cyph3D/GLObject/Material/MaterialMapDefinition.h"
#include "Cyph3D/GLObject/Material/MaterialMapType.h"

#include <map>

class ResourceManager;

class Material
{
public:
	const std::string& getName() const;
	void setName(std::string name);
	
	static void initialize();
	
	const GLTexture& getTexture(MaterialMapType mapType);
	
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