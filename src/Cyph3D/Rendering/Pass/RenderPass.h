#pragma once

#include "Cyph3D/PerfCounter/GpuPerfCounter.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

class GLTexture;
class Camera;
struct RenderRegistry;
struct PerfStep;

class RenderPass
{
public:
	RenderPass() = delete;
	RenderPass(std::unordered_map<std::string, GLTexture*>& textures, glm::ivec2 size, const char* name);
	virtual ~RenderPass() = default;
	
	glm::ivec2 getSize() const;
	
	PerfStep render(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& objects, Camera& camera);

protected:
	virtual void preparePipelineImpl() = 0;
	virtual void renderImpl(std::unordered_map<std::string, GLTexture*>& textures, RenderRegistry& objects, Camera& camera, PerfStep& previousFramePerfStep) = 0;
	virtual void restorePipelineImpl() = 0;
	
private:
	const char* _name;
	glm::ivec2 _size;
	GpuPerfCounter _perfCounter;
};