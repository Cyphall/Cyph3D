#pragma once

#include "Cyph3D/GLObject/Texture.h"
#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/PerfCounter/GpuPerfCounter.h"
#include <unordered_map>

class Camera;

class PostProcessingEffect
{
public:
	PostProcessingEffect(const char* name, glm::ivec2 size);
	virtual ~PostProcessingEffect() = default;
	
	glm::ivec2 getSize() const;
	
	std::pair<Texture*, PerfStep> render(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures, Camera& camera);

protected:
	virtual Texture* renderImpl(Texture* currentRenderTexture, std::unordered_map<std::string, Texture*>& textures, Camera& camera) = 0;

private:
	const char* _name;
	glm::ivec2 _size;
	GpuPerfCounter _perfCounter;
};