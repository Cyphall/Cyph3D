#pragma once

#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/Entity/Component/ModelRenderer.h"
#include "Cyph3D/Entity/Component/PointLight.h"

#include <vector>

class RenderRegistry
{
public:
	void clear();
	
	void addRenderRequest(const ModelRenderer::RenderData& renderData);
	void addRenderRequest(const DirectionalLight::RenderData& renderData);
	void addRenderRequest(const PointLight::RenderData& renderData);
	
	const std::vector<ModelRenderer::RenderData>& getModelRenderRequests() const;
	const std::vector<DirectionalLight::RenderData>& getDirectionalLightRenderRequests() const;
	const std::vector<PointLight::RenderData>& getPointLightRenderRequests() const;
	
private:
	std::vector<ModelRenderer::RenderData> _models;
	std::vector<DirectionalLight::RenderData> _directionalLights;
	std::vector<PointLight::RenderData> _pointLights;
};