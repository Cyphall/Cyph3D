#include "PostProcessingEffect.h"

#include "Cyph3D/PerfCounter/PerfStep.h"

PostProcessingEffect::PostProcessingEffect(const char* name, glm::uvec2 size):
_name(name), _size(size), _effectPerf(name)
{

}

GLTexture& PostProcessingEffect::render(GLTexture& input, Camera& camera, PerfStep& parentPerfStep)
{
	_effectPerf.clear();
	_effectPerf.setDuration(_perfCounter.retrieve());
	parentPerfStep.addSubstep(_effectPerf);
	
	glViewport(0, 0, _size.x, _size.y);
	
	_perfCounter.start();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, _name);
	GLTexture& output = renderImpl(input, camera);
	glPopDebugGroup();
	
	_perfCounter.stop();
	
	return output;
}