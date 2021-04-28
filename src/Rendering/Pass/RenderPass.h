#pragma once

#include <unordered_map>
#include "../../GLObject/Texture.h"
#include "../../Scene/SceneObject.h"
#include "../../Scene/Camera.h"
#include "../SceneObjectRegistry.h"

class RenderPass
{
public:
	RenderPass() = delete;
	RenderPass(std::unordered_map<std::string, Texture*>& textures, const char* name);
	
	void render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera);

protected:
	virtual void preparePipelineImpl() = 0;
	virtual void renderImpl(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) = 0;
	virtual void restorePipelineImpl() = 0;
	
private:
	const char* _name;
};
