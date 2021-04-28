#pragma once

#include "../../GLObject/Texture.h"
#include <unordered_map>

class PostProcessingEffect
{
public:
	PostProcessingEffect(const char* name);
	virtual ~PostProcessingEffect() = default;
	
	Texture* render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures);

protected:
	virtual Texture* renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures) = 0;

private:
	const char* _name;
};