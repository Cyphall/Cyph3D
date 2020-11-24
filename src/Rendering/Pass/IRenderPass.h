#pragma once

#include <unordered_map>
#include "../../GLObject/Texture.h"
#include "../../Scene/SceneObject.h"
#include "../../Scene/Camera.h"
#include "../SceneObjectRegistry.h"

class IRenderPass
{
public:
	IRenderPass() = delete;
	IRenderPass(std::unordered_map<std::string, Texture*>& textures){};
	
	virtual void preparePipeline() = 0;
	virtual void render(std::unordered_map<std::string, Texture*>& textures, SceneObjectRegistry& objects, Camera& camera) = 0;
	virtual void restorePipeline() = 0;
};
