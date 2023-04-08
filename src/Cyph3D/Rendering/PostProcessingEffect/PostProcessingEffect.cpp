#include "PostProcessingEffect.h"

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"

PostProcessingEffect::PostProcessingEffect(const char* name, glm::uvec2 size):
	_size(size), _name(name), _effectPerf(name)
{

}

const VKPtr<VKImageView>& PostProcessingEffect::render(const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImageView>& input, Camera& camera, PerfStep& parentPerfStep)
{
	_effectPerf.clear();
	_effectPerf.setDuration(_perfCounter.retrieve(commandBuffer));
	parentPerfStep.addSubstep(_effectPerf);
	
	_perfCounter.start(commandBuffer);
	
	commandBuffer->pushDebugGroup(_name);
	const VKPtr<VKImageView>& output = renderImpl(commandBuffer, input, camera);
	commandBuffer->popDebugGroup();
	
	_perfCounter.stop(commandBuffer);
	
	return output;
}