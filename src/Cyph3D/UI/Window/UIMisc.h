#pragma once

#include <glm/glm.hpp>

class PerfStep;

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
	
	static void displayPerfStep(const PerfStep& perfStep);
};