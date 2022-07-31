#pragma once

#include <vector>

struct PerfStep
{
	const char* name;
	double durationInMs;
	std::vector<PerfStep> subSteps;
};