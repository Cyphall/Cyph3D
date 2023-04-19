#pragma once

#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/Entity/Component/PointLight.h"
#include "Cyph3D/Entity/Component/ModelRenderer.h"

#include <vector>

struct RenderRegistry
{
	std::vector<ModelRenderer::RenderData> models;
	std::vector<DirectionalLight::RenderData> directionalLights;
	std::vector<PointLight::RenderData> pointLights;
	
	void clear()
	{
		models.clear();
		directionalLights.clear();
		pointLights.clear();
	}
};