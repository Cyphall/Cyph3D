#include "PostProcessingEffect.h"

#include "Cyph3D/PerfCounter/PerfStep.h"

PostProcessingEffect::PostProcessingEffect(const char* name, glm::ivec2 size):
_name(name), _size(size), _effectPerf(name)
{

}

std::pair<GLTexture*, const PerfStep&> PostProcessingEffect::render(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera)
{
	_effectPerf.clear();
	_effectPerf.setDuration(_perfCounter.retrieve());
	
	_perfCounter.start();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, _name);
	GLTexture* output = renderImpl(currentRenderTexture, textures, camera);
	glPopDebugGroup();
	
	_perfCounter.stop();
	
	return std::make_pair(output, std::ref(_effectPerf));
}

glm::ivec2 PostProcessingEffect::getSize() const
{
	return _size;
}