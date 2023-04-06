#pragma once

#include <string>
#include <string_view>
#include <vector>

class PerfStep
{
public:
	explicit PerfStep(std::string_view name);
	
	void clear();
	
	const std::string& getName() const;
	
	double getDuration() const;
	void setDuration(double duration);
	
	const std::vector<std::reference_wrapper<const PerfStep>>& getSubsteps() const;
	void addSubstep(const PerfStep& substep);
	
private:
	std::string _name;
	double _duration; // in ms
	std::vector<std::reference_wrapper<const PerfStep>> _substeps;
};