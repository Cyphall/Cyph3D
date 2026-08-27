#include "ShadowMapManager.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>

#include <CyphGPU/Image.hpp>
#include <ranges>

cgpu::ImagePtr c3d::ShadowMapManager::allocateDirectionalShadowMap(uint32_t resolution)
{
	ShadowMapContainer& container = _directionalShadowMaps[resolution];

	// all shadow maps for this resolution are already in use, create a new one
	if (container.allocatedShadowMaps == container.shadowMaps.size())
	{
		container.shadowMaps.push_back(
			cgpu::Image::create(
				Engine::getDeviceSession(),
				{
					.name = "Directional light shadow map",
					.format = SceneRenderer::DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT,
					.extent = {resolution, resolution, 1},
					.usages =
						vk::ImageUsageFlagBits::eDepthStencilAttachment |
						vk::ImageUsageFlagBits::eSampled,
				}
			)
		);
	}

	return container.shadowMaps[container.allocatedShadowMaps++];
}

cgpu::ImagePtr c3d::ShadowMapManager::allocatePointShadowMap(uint32_t resolution)
{
	ShadowMapContainer& container = _pointShadowMaps[resolution];

	// all shadow maps for this resolution are already in use, create a new one
	if (container.allocatedShadowMaps == container.shadowMaps.size())
	{
		container.shadowMaps.push_back(
			cgpu::Image::create(
				Engine::getDeviceSession(),
				{
					.name = "Point light shadow map",
					.format = SceneRenderer::POINT_SHADOW_MAP_DEPTH_FORMAT,
					.extent = {resolution, resolution, 1},
					.usages =
						vk::ImageUsageFlagBits::eDepthStencilAttachment |
						vk::ImageUsageFlagBits::eSampled,
					.layers = 6,
					.allow_cube_view = true,
				}
			)
		);
	}

	return container.shadowMaps[container.allocatedShadowMaps++];
}

void c3d::ShadowMapManager::resetDirectionalShadowMapAllocations()
{
	for (ShadowMapContainer& container : _directionalShadowMaps | std::views::values)
	{
		container.allocatedShadowMaps = 0;
	}
}

void c3d::ShadowMapManager::resetPointShadowMapAllocations()
{
	for (ShadowMapContainer& container : _pointShadowMaps | std::views::values)
	{
		container.allocatedShadowMaps = 0;
	}
}
