#pragma once

#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/PerfCounter/GpuPerfCounter.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

template<typename TInput, typename TOutput>
class RenderPass
{
public:
	RenderPass(glm::uvec2 size, const char* name):
		_name(name), _size(size), _renderPassPerf(name)
	{
	
	}
	
	virtual ~RenderPass() = default;
	
	TOutput render(TInput& input, PerfStep& parentPerfStep)
	{
		_renderPassPerf.clear();
		_renderPassPerf.setDuration(_perfCounter.retrieve());
		parentPerfStep.addSubstep(_renderPassPerf);
		
		glViewport(0, 0, _size.x, _size.y);
		
		_perfCounter.start();
		
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, _name);
		TOutput output = renderImpl(input);
		glPopDebugGroup();
		
		_perfCounter.stop();
		
		return output;
	}

protected:
	glm::uvec2 _size;
	PerfStep _renderPassPerf;
	
	virtual TOutput renderImpl(TInput& input) = 0;
	
private:
	const char* _name;
	GpuPerfCounter _perfCounter;
};