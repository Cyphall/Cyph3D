#pragma once

#include <memory>
#include <vector>

struct PerfStep
{
	const char* name;
	double durationInMs;
	std::vector<PerfStep> subSteps;
};