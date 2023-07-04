#include "RenderRegistry.h"

void RenderRegistry::clear()
{
	_models.clear();
	_directionalLights.clear();
	_pointLights.clear();
}

void RenderRegistry::addRenderRequest(const ModelRenderer::RenderData& renderData)
{
	_models.emplace_back(renderData);
}

void RenderRegistry::addRenderRequest(const DirectionalLight::RenderData& renderData)
{
	_directionalLights.emplace_back(renderData);
}

void RenderRegistry::addRenderRequest(const PointLight::RenderData& renderData)
{
	_pointLights.emplace_back(renderData);
}

const std::vector<ModelRenderer::RenderData>& RenderRegistry::getModelRenderRequests() const
{
	return _models;
}

const std::vector<DirectionalLight::RenderData>& RenderRegistry::getDirectionalLightRenderRequests() const
{
	return _directionalLights;
}

const std::vector<PointLight::RenderData>& RenderRegistry::getPointLightRenderRequests() const
{
	return _pointLights;
}
