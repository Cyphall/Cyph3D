#include "ShadowMapManager.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"

VKPtr<VKImage> ShadowMapManager::allocateDirectionalShadowMap(uint32_t resolution)
{
	ShadowMapContainer& container = _directionalShadowMaps[resolution];

	// all shadow maps for this resolution are already in use, create a new one
	if (container.allocatedShadowMaps == container.shadowMaps.size())
	{
		VKImageInfo imageInfo(
			SceneRenderer::DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT,
			glm::uvec2(resolution),
			1,
			1,
			vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		imageInfo.setName("Directional light shadow map");

		container.shadowMaps.push_back(VKImage::create(Engine::getVKContext(), imageInfo));
	}

	return container.shadowMaps[container.allocatedShadowMaps++];
}

VKPtr<VKImage> ShadowMapManager::allocatePointShadowMap(uint32_t resolution)
{
	ShadowMapContainer& container = _pointShadowMaps[resolution];

	// all shadow maps for this resolution are already in use, create a new one
	if (container.allocatedShadowMaps == container.shadowMaps.size())
	{
		VKImageInfo imageInfo(
			SceneRenderer::POINT_SHADOW_MAP_DEPTH_FORMAT,
			glm::uvec2(resolution),
			6,
			1,
			vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		imageInfo.enableCubeCompatibility();
		imageInfo.setName("Point light shadow map");

		container.shadowMaps.push_back(VKImage::create(Engine::getVKContext(), imageInfo));
	}

	return container.shadowMaps[container.allocatedShadowMaps++];
}

void ShadowMapManager::resetDirectionalShadowMapAllocations()
{
	for (auto& [resolution, container] : _directionalShadowMaps)
	{
		container.allocatedShadowMaps = 0;
	}
}

void ShadowMapManager::resetPointShadowMapAllocations()
{
	for (auto& [resolution, container] : _pointShadowMaps)
	{
		container.allocatedShadowMaps = 0;
	}
}