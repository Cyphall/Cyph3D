#pragma once

#include <unordered_map>
#include "../../GLObject/Texture.h"
#include "../../Scene/Camera.h"
#include "../RenderRegistry.h"

class RenderPass
{
public:
	RenderPass() = delete;
	RenderPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size, const char* name);
	
	glm::ivec2 getSize() const;
	
	void render(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera);

protected:
	virtual void preparePipelineImpl() = 0;
	virtual void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera) = 0;
	virtual void restorePipelineImpl() = 0;
	
private:
	const char* _name;
	glm::ivec2 _size;
};
