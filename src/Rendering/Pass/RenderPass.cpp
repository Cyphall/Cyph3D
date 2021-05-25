#include "RenderPass.h"

RenderPass::RenderPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size, const char* name):
_name(name), _size(size)
{

}

void RenderPass::render(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, _name);
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Prepare pipeline");
	preparePipelineImpl();
	glPopDebugGroup();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Render");
	renderImpl(textures, objects, camera);
	glPopDebugGroup();
	
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Restore pipeline");
	restorePipelineImpl();
	glPopDebugGroup();
	
	glPopDebugGroup();
}

glm::ivec2 RenderPass::getSize() const
{
	return _size;
}
