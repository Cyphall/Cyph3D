#include "LightBase.h"

LightBase::LightBase(Entity& entity):
Component(entity)
{
	setSrgbColor(glm::vec3(1));
}

glm::vec3 LightBase::getLinearColor() const
{
	return _linearColor;
}

void LightBase::setLinearColor(glm::vec3 color)
{
	_linearColor = color;
	_srgbColor = toSrgb(color);
}

glm::vec3 LightBase::getSrgbColor() const
{
	return _srgbColor;
}

void LightBase::setSrgbColor(glm::vec3 color)
{
	_srgbColor = color;
	_linearColor = toLinear(color);
}

glm::vec3 LightBase::toLinear(glm::vec3 color)
{
	glm::bvec3 cutoff = lessThan(color, glm::vec3(0.04045f));
	glm::vec3 higher = pow((color + glm::vec3(0.055f))/glm::vec3(1.055f), glm::vec3(2.4f));
	glm::vec3 lower = color/glm::vec3(12.92f);
	
	return glm::mix(higher, lower, cutoff);
}

glm::vec3 LightBase::toSrgb(glm::vec3 color)
{
	glm::bvec3 cutoff = lessThan(color, glm::vec3(0.0031308f));
	glm::vec3 higher = glm::vec3(1.055f)*pow(color, glm::vec3(1.0f/2.4f)) - glm::vec3(0.055f);
	glm::vec3 lower = color * glm::vec3(12.92f);
	
	return glm::mix(higher, lower, cutoff);
}

float LightBase::getIntensity() const
{
	return _intensity;
}

void LightBase::setIntensity(float intensity)
{
	_intensity = intensity;
}
