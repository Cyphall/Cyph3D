#include "Renderer.h"

void Renderer::render(RenderPass& pass, Camera& camera)
{
	pass.render(_textures, _registry, camera);
}

void Renderer::onNewFrame()
{
	_registry.clear();
}

void Renderer::requestShapeRendering(ShapeRenderer::RenderData request)
{
	_registry.shapes.push_back(request);
}

void Renderer::requestLightRendering(DirectionalLight::RenderData data)
{
	_registry.directionalLights.push_back(data);
}

void Renderer::requestLightRendering(PointLight::RenderData data)
{
	_registry.pointLights.push_back(data);
}
