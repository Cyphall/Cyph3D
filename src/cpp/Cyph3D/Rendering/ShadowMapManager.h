#pragma once

#include <CyphGPU/fwd.hpp>
#include <unordered_map>
#include <vector>

namespace c3d
{
class ShadowMapManager
{
public:
	cgpu::ImagePtr allocateDirectionalShadowMap(uint32_t resolution);
	cgpu::ImagePtr allocatePointShadowMap(uint32_t resolution);

	void resetDirectionalShadowMapAllocations();
	void resetPointShadowMapAllocations();

private:
	struct ShadowMapContainer
	{
		std::vector<cgpu::ImagePtr> shadowMaps;
		size_t allocatedShadowMaps = 0;
	};

	std::unordered_map<uint32_t, ShadowMapContainer> _directionalShadowMaps;
	std::unordered_map<uint32_t, ShadowMapContainer> _pointShadowMaps;
};
}
