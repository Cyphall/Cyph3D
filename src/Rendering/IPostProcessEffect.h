#pragma once

#include "../GLObject/Texture.h"
#include <unordered_map>

class IPostProcessEffect
{
public:
	virtual Texture* render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures) = 0;
};