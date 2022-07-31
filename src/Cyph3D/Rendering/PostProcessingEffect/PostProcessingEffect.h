#pragma once

#include "Cyph3D/PerfCounter/GpuPerfCounter.h"

#include <glm/glm.hpp>
#include <unordered_map>

class Camera;
class Texture;
struct PerfStep;

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