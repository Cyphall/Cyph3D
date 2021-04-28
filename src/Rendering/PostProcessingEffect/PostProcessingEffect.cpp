#include "PostProcessingEffect.h"

PostProcessingEffect::PostProcessingEffect(const char* name):
_name(name)
{

}

Texture* PostProcessingEffect::render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, _name);
	Texture* output = renderImpl(currentRenderTexture, textures);
	glPopDebugGroup();
	
	return output;
}
