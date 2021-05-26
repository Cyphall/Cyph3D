#include "PostProcessingEffect.h"

PostProcessingEffect::PostProcessingEffect(const char* name, glm::ivec2 size):
_name(name), _size(size)
{

}

Texture* PostProcessingEffect::render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures, Camera& camera)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, _name);
	Texture* output = renderImpl(currentRenderTexture, textures, camera);
	glPopDebugGroup();
	
	return output;
}

glm::ivec2 PostProcessingEffect::getSize() const
{
	return _size;
}
