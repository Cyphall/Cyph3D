#pragma once

#include "Cyph3D/VKObject/Image/VKImageView.h"

#include <unordered_map>
#include <vector>
#include <array>

class ShadowMapManager
{
public:
	struct DirectionalShadowMapData
	{
		VKPtr<VKImageView> imageView;
	};
	
	struct PointShadowMapData
	{
		VKPtr<VKImageView> imageViewAllLayers;
		std::array<VKPtr<VKImageView>, 6> imageViewsOneLayer;
	};
	
	DirectionalShadowMapData allocateDirectionalShadowMap(uint32_t resolution);
	PointShadowMapData allocatePointShadowMap(uint32_t resolution);
	
	void resetDirectionalShadowMapAllocations();
	void resetPointShadowMapAllocations();
	
private:
	template<typename T>
	struct ShadowMapContainer
	{
		std::vector<T> shadowMaps;
		size_t allocatedShadowMaps = 0;
	};
	
	std::unordered_map<uint32_t, ShadowMapContainer<DirectionalShadowMapData>> _directionalShadowMaps;
	std::unordered_map<uint32_t, ShadowMapContainer<PointShadowMapData>> _pointShadowMaps;
};