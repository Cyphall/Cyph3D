#pragma once

#include <unordered_map>
#include "Cyph3D/GLObject/Texture.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/PerfCounter/GpuPerfCounter.h"

class RenderPass
{
public:
	RenderPass() = delete;
	RenderPass(std::unordered_map<std::string, Texture*>& textures, glm::ivec2 size, const char* name);
	virtual ~RenderPass() = default;
	
	glm::ivec2 getSize() const;
	
	PerfStep render(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera);

protected:
	virtual void preparePipelineImpl() = 0;
	virtual void renderImpl(std::unordered_map<std::string, Texture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep) = 0;
	virtual void restorePipelineImpl() = 0;
	
private:
	const char* _name;
	glm::ivec2 _size;
	GpuPerfCounter _perfCounter;
};