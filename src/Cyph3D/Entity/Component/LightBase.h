#pragma once

#include "Cyph3D/Entity/Component/Component.h"

#include <glm/glm.hpp>

class LightBase : public Component
{
public:
	explicit LightBase(Entity& entity);

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
};