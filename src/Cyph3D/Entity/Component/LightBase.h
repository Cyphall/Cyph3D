#pragma once

#include "Component.h"
#include <glm/glm.hpp>

class LightBase : public Component
{
public:
	LightBase(Entity& entity);
	
	glm::vec3 getLinearColor() const;
	void setLinearColor(glm::vec3 color);
	
	glm::vec3 getSrgbColor() const;
	void setSrgbColor(glm::vec3 color);
	
	float getIntensity() const;
	void setIntensity(float intensity);

private:
	glm::vec3 _linearColor;
	glm::vec3 _srgbColor;
	float _intensity = 1;
	
	static glm::vec3 toLinear(glm::vec3 color);
	static glm::vec3 toSrgb(glm::vec3 color);
};
