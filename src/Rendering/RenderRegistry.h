#pragma once

#include <vector>
#include "../Entity/Component/DirectionalLight.h"
#include "../Entity/Component/PointLight.h"
#include "../Entity/Component/MeshRenderer.h"

struct RenderRegistry
{
	std::vector<MeshRenderer::RenderData> meshes;
	std::vector<DirectionalLight::RenderData> directionalLights;
	std::vector<PointLight::RenderData> pointLights;
};