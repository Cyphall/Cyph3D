#pragma once

#include <vector>
#include "../Entity/Component/DirectionalLight.h"
#include "../Entity/Component/PointLight.h"
#include "../Entity/Component/ShapeRenderer.h"

struct RenderRegistry
{
	std::vector<ShapeRenderer::RenderData> shapes;
	std::vector<DirectionalLight::RenderData> directionalLights;
	std::vector<PointLight::RenderData> pointLights;
	
	void clear()
	{
		shapes.clear();
		directionalLights.clear();
		pointLights.clear();
	}
};