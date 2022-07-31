#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct PerfStep;

class UIMisc
{
public:
	static void show();

private:
	static glm::ivec2 _resolution;
	
	static void displayPerfStep(const PerfStep& perfStep);
};