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
	PostProcessingEffect(const char* name, glm::uvec2 size);
	virtual ~PostProcessingEffect() = default;
	
	GLTexture& render(GLTexture& input, Camera& camera, PerfStep& parentPerfStep);

protected:
	glm::uvec2 _size;
	
	virtual GLTexture& renderImpl(GLTexture& input, Camera& camera) = 0;

private:
	const char* _name;
	PerfStep _effectPerf;
	GpuPerfCounter _perfCounter;
};