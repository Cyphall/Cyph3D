#pragma once

#include "Cyph3D/VKObject/Image/VKImageView.h"

#include <unordered_map>
#include <vector>

class ShadowMapManager
{
public:
	VKPtr<VKImageView> allocateDirectionalShadowMap(uint32_t resolution);
	VKPtr<VKImageView> allocatePointShadowMap(uint32_t resolution);
	
	void resetDirectionalShadowMapAllocations();
	void resetPointShadowMapAllocations();
	
private:
	struct ShadowMapContainer
	{
		std::vector<VKPtr<VKImageView>> shadowMaps;
		size_t allocatedShadowMaps = 0;
	};
	
	std::unordered_map<uint32_t, ShadowMapContainer> _directionalShadowMaps;
	std::unordered_map<uint32_t, ShadowMapContainer> _pointShadowMaps;
};