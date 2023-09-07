#include "ShadowMapManager.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"

VKPtr<VKImageView> ShadowMapManager::allocateDirectionalShadowMap(uint32_t resolution)
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

		VKPtr<VKImage> shadowMap = VKImage::create(Engine::getVKContext(), imageInfo);

		VKImageViewInfo imageViewInfo(
			shadowMap,
			vk::ImageViewType::e2D);
		
		container.shadowMaps.push_back(VKImageView::create(Engine::getVKContext(), imageViewInfo));
	}
	
	return container.shadowMaps[container.allocatedShadowMaps++];
}

VKPtr<VKImageView> ShadowMapManager::allocatePointShadowMap(uint32_t resolution)
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
		
		VKPtr<VKImage> shadowMap = VKImage::create(Engine::getVKContext(), imageInfo);
		
		VKImageViewInfo imageViewInfo(
			shadowMap,
			vk::ImageViewType::eCube);
		
		container.shadowMaps.push_back(VKImageView::create(Engine::getVKContext(), imageViewInfo));
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