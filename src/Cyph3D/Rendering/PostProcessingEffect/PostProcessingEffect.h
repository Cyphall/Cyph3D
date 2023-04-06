#pragma once

#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/PerfCounter/GpuPerfCounter.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

class Camera;
class GLTexture;

class PostProcessingEffect
{
public:
	PostProcessingEffect(const char* name, glm::ivec2 size);
	virtual ~PostProcessingEffect() = default;
	
	glm::ivec2 getSize() const;
	
	std::pair<GLTexture*, const PerfStep&> render(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera);

protected:
	virtual GLTexture* renderImpl(GLTexture* currentRenderTexture, std::unordered_map<std::string, GLTexture*>& textures, Camera& camera) = 0;

private:
	const char* _name;
	glm::ivec2 _size;
	PerfStep _effectPerf;
	GpuPerfCounter _perfCounter;
};