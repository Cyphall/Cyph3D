#pragma once

#include <glm/glm.hpp>

class UIMisc
{
public:
	static void show();

	static bool isSimulationEnabled();
	static int viewportSampleCount();

private:
	static glm::ivec2 _resolution;
	static uint32_t _renderSampleCount;
	static bool _simulationEnabled;
	static int _viewportSampleCount;

	static std::array<float, 512> _frametimes;
	static uint32_t _lastFrametimeIndex;
	static float _overlayFrametime;
	static float _timeUntilOverlayUpdate;

	static void displayFrametime();
};