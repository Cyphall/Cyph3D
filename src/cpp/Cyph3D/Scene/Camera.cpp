#include "Camera.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/MathHelper.h>
#include <Cyph3D/Window.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::vec3 c3d::Camera::getOrientation() const
{
	if (_orientationChanged)
		recalculateOrientation();
	return _orientation;
}

glm::vec3 c3d::Camera::getSideOrientation() const
{
	if (_orientationChanged)
		recalculateOrientation();
	return _sideOrientation;
}

glm::mat4 c3d::Camera::getView() const
{
	if (_viewChanged)
		recalculateView();
	return _view;
}

glm::mat4 c3d::Camera::getProjection() const
{
	if (_projectionChanged)
		recalculateProjection();
	return _projection;
}

glm::vec3 c3d::Camera::getPosition() const
{
	return _position;
}

void c3d::Camera::setPosition(glm::vec3 position)
{
	_position = position;
	viewChanged();
}

glm::vec2 c3d::Camera::getSphericalCoords() const
{
	return _sphericalCoords;
}

void c3d::Camera::setSphericalCoords(glm::vec2 sphericalCoords)
{
	_sphericalCoords = glm::vec2(
		glm::mod(sphericalCoords.x, 360.0f),
		glm::clamp(sphericalCoords.y, -89.0f, 89.0f)
	);
	orientationChanged();
}

float c3d::Camera::getSpeed() const
{
	return _speed;
}

void c3d::Camera::setSpeed(float speed)
{
	_speed = speed;
}

float c3d::Camera::getExposure() const
{
	return _exposure;
}

void c3d::Camera::setExposure(float exposure)
{
	_exposure = exposure;
}

float c3d::Camera::getVerticalFov() const
{
	return _verticalFov;
}

void c3d::Camera::setVerticalFov(float vfov)
{
	_verticalFov = vfov;
	projectionChanged();
}

void c3d::Camera::setHorizontalFov(float hfov, float referenceAspectRatio)
{
	_verticalFov = MathHelper::fovXtoY(hfov, referenceAspectRatio);
	projectionChanged();
}

c3d::Camera::Camera(glm::vec3 position, glm::vec2 sphericalCoords):
	_position(position),
	_sphericalCoords(sphericalCoords)
{
	setHorizontalFov(80.0f, 16.0f / 9.0f);
	setAspectRatio(16.0f / 9.0f);
}

bool c3d::Camera::update(glm::vec2 mousePosDelta)
{
	float ratio = static_cast<float>(Engine::getTimer().deltaTime()) * _speed;

	bool changed = false;

	if (Engine::getWindow().getKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		ratio /= 20;
	}
	if (Engine::getWindow().getKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		ratio *= 5;
	}

	if (Engine::getWindow().getKey(GLFW_KEY_W) == GLFW_PRESS)
	{
		setPosition(getPosition() + getOrientation() * ratio);
		changed = true;
	}
	if (Engine::getWindow().getKey(GLFW_KEY_S) == GLFW_PRESS)
	{
		setPosition(getPosition() - getOrientation() * ratio);
		changed = true;
	}
	if (Engine::getWindow().getKey(GLFW_KEY_A) == GLFW_PRESS)
	{
		setPosition(getPosition() - getSideOrientation() * ratio);
		changed = true;
	}
	if (Engine::getWindow().getKey(GLFW_KEY_D) == GLFW_PRESS)
	{
		setPosition(getPosition() + getSideOrientation() * ratio);
		changed = true;
	}

	glm::vec2 sphericalCoords = getSphericalCoords();
	sphericalCoords.x += mousePosDelta.x / 12.0f;
	sphericalCoords.y -= mousePosDelta.y / 12.0f;
	setSphericalCoords(sphericalCoords);

	if (mousePosDelta.x != 0.0f || mousePosDelta.y != 0.0f)
	{
		changed = true;
	}

	return changed;
}

void c3d::Camera::recalculateOrientation() const
{
	glm::vec2 sphericalCoordsRadians = glm::radians(_sphericalCoords);

	_orientation = glm::vec3(
		glm::sin(sphericalCoordsRadians.x) * glm::cos(sphericalCoordsRadians.y),
		glm::sin(sphericalCoordsRadians.y),
		-glm::cos(sphericalCoordsRadians.x) * glm::cos(sphericalCoordsRadians.y)
	);

	_sideOrientation = glm::vec3(
		glm::cos(sphericalCoordsRadians.x),
		0,
		glm::sin(sphericalCoordsRadians.x)
	);

	_orientationChanged = false;
}

void c3d::Camera::recalculateView() const
{
	_view = glm::lookAt(getPosition(), getPosition() + getOrientation(), glm::vec3(0, 1, 0));

	_viewChanged = false;
}

void c3d::Camera::recalculateProjection() const
{
	_projection = glm::perspective(_verticalFov, _aspectRatio, NEAR_DISTANCE, FAR_DISTANCE);
	_projection[1][1] *= -1;

	_projectionChanged = false;
}

float c3d::Camera::getAspectRatio() const
{
	return _aspectRatio;
}

void c3d::Camera::setAspectRatio(float aspectRatio)
{
	_aspectRatio = aspectRatio;
	projectionChanged();
}

const std::array<glm::vec3, 4>& c3d::Camera::getCornerRays() const
{
	if (_cornerRaysChanged)
		recalculateCornerRays();
	return _cornerRays;
}

void c3d::Camera::viewChanged() const
{
	_viewChanged = true;
	cornerRaysChanged();
}

void c3d::Camera::projectionChanged() const
{
	_projectionChanged = true;
	cornerRaysChanged();
}

void c3d::Camera::cornerRaysChanged() const
{
	_cornerRaysChanged = true;
}

void c3d::Camera::recalculateCornerRays() const
{
	glm::mat4 projInverse = glm::inverse(getProjection());
	glm::mat4 viewInverse = glm::affineInverse(getView());

	_cornerRays[0] = glm::vec3(-1, -1, 1); // top left
	_cornerRays[1] = glm::vec3(1, -1, 1); // top right
	_cornerRays[2] = glm::vec3(-1, 1, 1); // bottom left
	_cornerRays[3] = glm::vec3(1, 1, 1); // bottom right

	for (int i = 0; i < 4; i++)
	{
		glm::vec4 vec = projInverse * glm::vec4(_cornerRays[i], 1);
		vec /= vec.w;
		vec.w = 0;
		vec = viewInverse * vec;
		_cornerRays[i] = glm::normalize(glm::vec3(vec));
	}

	_cornerRaysChanged = false;
}

void c3d::Camera::orientationChanged() const
{
	_orientationChanged = true;
	viewChanged();
}