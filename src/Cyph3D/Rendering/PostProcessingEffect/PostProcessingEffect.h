#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/PerfCounter/GpuPerfCounter.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

class Camera;
class VKImageView;
class VKCommandBuffer;

class PostProcessingEffect
{
public:
	PostProcessingEffect(const char* name, glm::uvec2 size);
	virtual ~PostProcessingEffect() = default;
	
	const VKPtr<VKImageView>& render(const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImageView>& input, Camera& camera, PerfStep& parentPerfStep);
	void resize(glm::uvec2 size);

protected:
	glm::uvec2 _size;
	
	virtual const VKPtr<VKImageView>& onRender(const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImageView>& input, Camera& camera) = 0;
	virtual void onResize() = 0;

private:
	const char* _name;
	PerfStep _effectPerf;
	GpuPerfCounter _perfCounter;
};