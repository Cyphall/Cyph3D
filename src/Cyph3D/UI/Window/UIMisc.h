#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "../../PerfCounter/PerfStep.h"

class UIMisc
{
public:
	static void show();

private:
	static glm::ivec2 _resolution;
	
	static void displayPerfStep(const PerfStep& perfStep);
};
