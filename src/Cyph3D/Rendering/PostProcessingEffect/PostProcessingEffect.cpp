#include "PostProcessingEffect.h"

#include "Cyph3D/PerfCounter/PerfStep.h"

PostProcessingEffect::PostProcessingEffect(const char* name, glm::ivec2 size):
_name(name), _size(size)
{

}

std::pair<GLTexture*, PerfStep> PostProcessingEffect::render(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera)
{
	PerfStep perfStep{};
	perfStep.name = _name;
	perfStep.durationInMs = _perfCounter.retrieve();
	
	_perfCounter.start();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, _name);
	GLTexture* output = renderImpl(currentRenderTexture, textures, camera);
	glPopDebugGroup();
	
	_perfCounter.stop();
	
	return std::make_pair(output, perfStep);
}

glm::ivec2 PostProcessingEffect::getSize() const
{
	return _size;
}