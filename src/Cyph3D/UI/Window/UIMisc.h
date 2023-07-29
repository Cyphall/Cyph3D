#pragma once

#include <glm/glm.hpp>

class PerfStep;

class UIMisc
{
public:
	static void show();
	
	static bool isSimulationEnabled();

private:
	static glm::ivec2 _resolution;
	static int _sampleCount;
	static bool _simulationEnabled;
	
	static void displayPerfStep(const PerfStep& perfStep);
};