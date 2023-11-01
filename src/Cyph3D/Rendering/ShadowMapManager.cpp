#include "ShadowMapManager.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"

ShadowMapManager::DirectionalShadowMapData ShadowMapManager::allocateDirectionalShadowMap(uint32_t resolution)
{
	ShadowMapContainer<DirectionalShadowMapData>& container = _directionalShadowMaps[resolution];
	
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

		VKPtr<VKImage> shadowMap = VKImage::create(Engine::getVKContext(), imageInfo);

		VKImageViewInfo imageViewInfo(
			shadowMap,
			vk::ImageViewType::e2D);
		
		DirectionalShadowMapData data{
			.imageView = VKImageView::create(Engine::getVKContext(), imageViewInfo)
		};
		
		container.shadowMaps.push_back(data);
	}
	
	return container.shadowMaps[container.allocatedShadowMaps++];
}

ShadowMapManager::PointShadowMapData ShadowMapManager::allocatePointShadowMap(uint32_t resolution)
{
	ShadowMapContainer<PointShadowMapData>& container = _pointShadowMaps[resolution];
	
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
		
		VKPtr<VKImage> shadowMap = VKImage::create(Engine::getVKContext(), imageInfo);
		
		PointShadowMapData data;
		
		{
			VKImageViewInfo imageViewInfo(
				shadowMap,
				vk::ImageViewType::eCube);
			
			data.imageViewAllLayers = VKImageView::create(Engine::getVKContext(), imageViewInfo);
		}
		
		for (int i = 0; i < 6; i++)
		{
			VKImageViewInfo imageViewInfo(
				shadowMap,
				vk::ImageViewType::e2D);
			
			imageViewInfo.setCustomLayerRange({i, i});
			
			data.imageViewsOneLayer[i] = VKImageView::create(Engine::getVKContext(), imageViewInfo);
		}
		
		container.shadowMaps.push_back(data);
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