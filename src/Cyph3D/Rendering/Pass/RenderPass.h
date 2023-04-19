#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
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
	
	TOutput render(const VKPtr<VKCommandBuffer>& commandBuffer, TInput& input, PerfStep& parentPerfStep)
	{
		_renderPassPerf.clear();
		_renderPassPerf.setDuration(_perfCounter.retrieve(commandBuffer));
		parentPerfStep.addSubstep(_renderPassPerf);
		
		_perfCounter.start(commandBuffer);
		
		commandBuffer->pushDebugGroup(_name);
		TOutput output = onRender(commandBuffer, input);
		commandBuffer->popDebugGroup();
		
		_perfCounter.stop(commandBuffer);
		
		return output;
	}
	
	void resize(glm::uvec2 size)
	{
		_size = size;
		onResize();
	}

protected:
	glm::uvec2 _size;
	PerfStep _renderPassPerf;
	
	virtual TOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, TInput& input) = 0;
	virtual void onResize() = 0;
	
private:
	const char* _name;
	GpuPerfCounter _perfCounter;
};