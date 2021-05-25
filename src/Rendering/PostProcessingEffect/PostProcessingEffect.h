#pragma once

#include "../../GLObject/Texture.h"
#include <unordered_map>

class PostProcessingEffect
{
public:
	PostProcessingEffect(const char* name, glm::ivec2 size);
	virtual ~PostProcessingEffect() = default;
	
	glm::ivec2 getSize() const;
	
	Texture* render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures);

protected:
	virtual Texture* renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures) = 0;

private:
	const char* _name;
	glm::ivec2 _size;
};