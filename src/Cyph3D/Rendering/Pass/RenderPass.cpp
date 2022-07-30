#include "Cyph3D/Rendering/Pass/RenderPass.h"

RenderPass::RenderPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size, const char* name):
_name(name), _size(size)
{

}

PerfStep RenderPass::render(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera)
{
	PerfStep perfStep{};
	perfStep.name = _name;
	perfStep.durationInMs = _perfCounter.retrieve();
	
	_perfCounter.start();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, _name);
	
	glm::ivec2 size = getSize();
	glViewport(0, 0, size.x, size.y);
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Prepare pipeline");
	preparePipelineImpl();
	glPopDebugGroup();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Render");
	renderImpl(textures, objects, camera, perfStep);
	glPopDebugGroup();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Restore pipeline");
	restorePipelineImpl();
	glPopDebugGroup();
	
	glPopDebugGroup();
	
	_perfCounter.stop();
	
	return perfStep;
}

glm::ivec2 RenderPass::getSize() const
{
	return _size;
}