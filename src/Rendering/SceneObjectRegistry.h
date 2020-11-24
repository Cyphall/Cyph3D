#pragma once

#include <vector>
#include "../Scene/MeshObject.h"
#include "../Scene/DirectionalLight.h"
#include "../Scene/PointLight.h"

struct SceneObjectRegistry
{
	std::vector<MeshObject*> meshObjects;
	std::vector<DirectionalLight*> directionalLights;
	std::vector<PointLight*> pointLights;
};