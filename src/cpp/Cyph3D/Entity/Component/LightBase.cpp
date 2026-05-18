#include "LightBase.h"

#include "Cyph3D/Helper/MathHelper.h"

c3d::LightBase::LightBase(Entity& entity):
	Component(entity)
{
	setSrgbColor(glm::vec3(1));
}

glm::vec3 c3d::LightBase::getLinearColor() const
{
	return _linearColor;
}

void c3d::LightBase::setLinearColor(glm::vec3 color)
{
	_linearColor = color;
	_srgbColor = MathHelper::linearToSrgb(color);

	_changed();
}

glm::vec3 c3d::LightBase::getSrgbColor() const
{
	return _srgbColor;
}

void c3d::LightBase::setSrgbColor(glm::vec3 color)
{
	_srgbColor = color;
	_linearColor = MathHelper::srgbToLinear(color);

	_changed();
}

float c3d::LightBase::getIntensity() const
{
	return _intensity;
}

void c3d::LightBase::setIntensity(float intensity)
{
	_intensity = intensity;

	_changed();
}