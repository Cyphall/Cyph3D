#pragma once

#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/Entity/Component/PointLight.h"
#include "Cyph3D/Entity/Component/ShapeRenderer.h"

#include <vector>

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