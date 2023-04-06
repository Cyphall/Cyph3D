#include "RenderPass.h"

#include "Cyph3D/PerfCounter/PerfStep.h"

RenderPass::RenderPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size, const char* name):
_name(name), _size(size), _renderPassPerf(name)
{

}

const PerfStep& RenderPass::render(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& objects, Camera& camera)
{
	_renderPassPerf.clear();
	_renderPassPerf.setDuration(_perfCounter.retrieve());
	
	_perfCounter.start();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, _name);
	
	glm::ivec2 size = getSize();
	glViewport(0, 0, size.x, size.y);
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Prepare pipeline");
	preparePipelineImpl();
	glPopDebugGroup();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Render");
	renderImpl(textures, objects, camera, _renderPassPerf);
	glPopDebugGroup();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Restore pipeline");
	restorePipelineImpl();
	glPopDebugGroup();
	
	glPopDebugGroup();
	
	_perfCounter.stop();
	
	return _renderPassPerf;
}

glm::ivec2 RenderPass::getSize() const
{
	return _size;
}