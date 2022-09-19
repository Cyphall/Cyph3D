#pragma once

#include "Cyph3D/GLObject/Material/MaterialMap.h"
#include "Cyph3D/GLObject/Material/MaterialMapDefinition.h"
#include "Cyph3D/GLObject/Material/MaterialMapType.h"

#include <map>

class ResourceManager;

class Material
{
public:
	const std::string& getPath() const;
	
	static void initialize();
	
	const GLTexture& getTexture(MaterialMapType mapType);
	
	static Material* getDefault();
	static Material* getMissing();

private:
	friend class ResourceManager;

	explicit Material(const std::string& path, ResourceManager* resourceManager);
	
	std::map<MaterialMapType, MaterialMap> _maps;
	std::string _path;
	
	static Material* _default;
	static Material* _missing;
	
	static std::map<MaterialMapType, MaterialMapDefinition> _mapDefinitions;
};