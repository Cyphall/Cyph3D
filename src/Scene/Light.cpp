#include "Light.h"

glm::vec3 Light::getLinearColor() const
{
	return _linearColor;
}

void Light::setLinearColor(glm::vec3 color)
{
	_linearColor = color;
	_srgbColor = toSrgb(color);
}

glm::vec3 Light::getSrgbColor() const
{
	return _srgbColor;
}

void Light::setSrgbColor(glm::vec3 color)
{
	_srgbColor = color;
	_linearColor = toLinear(color);
}

glm::vec3 Light::toLinear(glm::vec3 color)
{
	glm::bvec3 cutoff = lessThan(color, glm::vec3(0.04045));
	glm::vec3 higher = pow((color + glm::vec3(0.055))/glm::vec3(1.055), glm::vec3(2.4));
	glm::vec3 lower = color/glm::vec3(12.92);
	
	return glm::mix(higher, lower, cutoff);
}

glm::vec3 Light::toSrgb(glm::vec3 color)
{
	glm::bvec3 cutoff = lessThan(color, glm::vec3(0.0031308));
	glm::vec3 higher = glm::vec3(1.055)*pow(color, glm::vec3(1.0/2.4)) - glm::vec3(0.055);
	glm::vec3 lower = color * glm::vec3(12.92);
	
	return glm::mix(higher, lower, cutoff);
}

float Light::getIntensity() const
{
	return _intensity;
}

void Light::setIntensity(float intensity)
{
	_intensity = intensity;
}

Light::Light(Transform* parent, const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 srgbColor, float intensity):
SceneObject(parent, name, position, rotation, scale), _srgbColor(srgbColor), _linearColor(toLinear(srgbColor)), _intensity(intensity)
{

}

Light::Light(Transform* parent, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec3 srgbColor, float intensity):
SceneObject(parent, name, position, rotation, scale), _srgbColor(srgbColor), _linearColor(toLinear(srgbColor)), _intensity(intensity)
{

}
