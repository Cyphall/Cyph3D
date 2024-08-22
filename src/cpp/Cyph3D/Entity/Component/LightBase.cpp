#include "LightBase.h"

#include "Cyph3D/Helper/MathHelper.h"

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
	_srgbColor = MathHelper::linearToSrgb(color);

	_changed();
}

glm::vec3 LightBase::getSrgbColor() const
{
	return _srgbColor;
}

void LightBase::setSrgbColor(glm::vec3 color)
{
	_srgbColor = color;
	_linearColor = MathHelper::srgbToLinear(color);

	_changed();
}

float LightBase::getIntensity() const
{
	return _intensity;
}

void LightBase::setIntensity(float intensity)
{
	_intensity = intensity;

	_changed();
}