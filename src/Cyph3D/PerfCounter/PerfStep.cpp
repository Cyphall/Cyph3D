#include "PerfStep.h"

PerfStep::PerfStep(std::string_view name):
	_name(name)
{

}

const std::string& PerfStep::getName() const
{
	return _name;
}

void PerfStep::clear()
{
	_duration = 0;
	_substeps.clear();
}

double PerfStep::getDuration() const
{
	return _duration;
}

void PerfStep::setDuration(double duration)
{
	_duration = duration;
}

const std::vector<std::reference_wrapper<const PerfStep>>& PerfStep::getSubsteps() const
{
	return _substeps;
}

void PerfStep::addSubstep(const PerfStep& substep)
{
	_substeps.push_back(std::ref(substep));
}