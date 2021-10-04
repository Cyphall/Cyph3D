#pragma once

#include <vector>
#include <memory>

struct PerfStep
{
	const char* name;
	double durationInMs;
	std::vector<PerfStep> subSteps;
};