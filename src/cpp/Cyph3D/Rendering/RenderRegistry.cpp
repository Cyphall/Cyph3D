#include "RenderRegistry.h"

void c3d::RenderRegistry::clear()
{
	_models.clear();
	_directionalLights.clear();
	_pointLights.clear();
}

void c3d::RenderRegistry::addRenderRequest(const ModelRenderer::RenderData& renderData)
{
	_models.emplace_back(renderData);
}

void c3d::RenderRegistry::addRenderRequest(const DirectionalLight::RenderData& renderData)
{
	_directionalLights.emplace_back(renderData);
}

void c3d::RenderRegistry::addRenderRequest(const PointLight::RenderData& renderData)
{
	_pointLights.emplace_back(renderData);
}

const std::vector<c3d::ModelRenderer::RenderData>& c3d::RenderRegistry::getModelRenderRequests() const
{
	return _models;
}

const std::vector<c3d::DirectionalLight::RenderData>& c3d::RenderRegistry::getDirectionalLightRenderRequests() const
{
	return _directionalLights;
}

const std::vector<c3d::PointLight::RenderData>& c3d::RenderRegistry::getPointLightRenderRequests() const
{
	return _pointLights;
}