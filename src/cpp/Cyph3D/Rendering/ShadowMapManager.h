#pragma once

#include "Cyph3D/VKObject/Image/VKImage.h"

#include <unordered_map>
#include <vector>

class ShadowMapManager
{
public:
	std::shared_ptr<VKImage> allocateDirectionalShadowMap(uint32_t resolution);
	std::shared_ptr<VKImage> allocatePointShadowMap(uint32_t resolution);

	void resetDirectionalShadowMapAllocations();
	void resetPointShadowMapAllocations();

private:
	struct ShadowMapContainer
	{
		std::vector<std::shared_ptr<VKImage>> shadowMaps;
		size_t allocatedShadowMaps = 0;
	};

	std::unordered_map<uint32_t, ShadowMapContainer> _directionalShadowMaps;
	std::unordered_map<uint32_t, ShadowMapContainer> _pointShadowMaps;
};