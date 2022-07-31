#pragma once

#include "Cyph3D/Enums/ResourceType.h"

#include <string>
#include <vector>

class UIResourceExplorer
{
public:
	static void init();
	static void show();
	static void rescanFiles();

private:
	static std::vector<std::string> _meshes;
	static std::vector<std::string> _materials;
	static std::vector<std::string> _skyboxes;
	
	static ResourceType _currentResourceType;
	
	static void findMeshes(const std::string& path);
	static void findMaterial(const std::string& path);
	static void findSkyboxes(const std::string& path);
};