#include "Camera.h"
#include "../Helper/MathHelper.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../Engine.h"
#include "../Window.h"

glm::vec3 Camera::getOrientation()
{
	if (_orientationChanged) recalculateOrientation();
	return _orientation;
}

glm::vec3 Camera::getSideOrientation()
{
	if (_orientationChanged) recalculateOrientation();
	return _sideOrientation;
}

glm::mat4 Camera::getView()
{
	if (_viewChanged) recalculateView();
	return _view;
}

glm::mat4 Camera::getProjection()
{
	if (_projectionChanged) recalculateProjection();
	return _projection;
}

glm::vec3 Camera::getPosition() const
{
	return _position;
}

void Camera::setPosition(glm::vec3 position)
{
	_position = position;
	viewChanged();
}

glm::vec2 Camera::getSphericalCoords() const
{
	return _sphericalCoords;
}

void Camera::setSphericalCoords(glm::vec2 sphericalCoords)
{
	_sphericalCoords = sphericalCoords;
	_orientationChanged = true;
	viewChanged();
}

float Camera::getSpeed() const
{
	return _speed;
}

void Camera::setSpeed(float speed)
{
	_speed = speed;
}

float Camera::getExposure() const
{
	return _exposure;
}

void Camera::setExposure(float exposure)
{
	_exposure = exposure;
}

float Camera::getVerticalFov() const
{
	return _verticalFov;
}

void Camera::setVerticalFov(float vfov)
{
	_verticalFov = vfov;
	projectionChanged();
}

void Camera::setHorizontalFov(float hfov, float referenceAspectRatio)
{
	_verticalFov = MathHelper::fovXtoY(hfov, referenceAspectRatio);
	projectionChanged();
}

Camera::Camera(glm::vec3 position, glm::vec2 sphericalCoords):
	_position(position), _sphericalCoords(sphericalCoords)
{
	setHorizontalFov(80, 16.0/9.0);
	setAspectRatio(16.0/9.0);
}

void Camera::update(glm::vec2 mousePosDelta)
{
	float ratio = (float)Engine::getTimer().deltaTime() * _speed;
	
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
	}
	if (Engine::getWindow().getKey(GLFW_KEY_S) == GLFW_PRESS)
	{
		setPosition(getPosition() - getOrientation() * ratio);
	}
	if (Engine::getWindow().getKey(GLFW_KEY_A) == GLFW_PRESS)
	{
		setPosition(getPosition() + getSideOrientation() * ratio);
	}
	if (Engine::getWindow().getKey(GLFW_KEY_D) == GLFW_PRESS)
	{
		setPosition(getPosition() - getSideOrientation() * ratio);
	}
	
	setSphericalCoords(getSphericalCoords() - mousePosDelta / 12.0f);
}

void Camera::recalculateOrientation()
{
	_sphericalCoords = glm::vec2(
		glm::mod(_sphericalCoords.x, 360.0f),
		glm::clamp(_sphericalCoords.y, -89.0f, 89.0f)
	);
	
	glm::vec2 sphericalCoordsRadians = glm::radians(_sphericalCoords);
	
	_orientation = glm::vec3(
		glm::cos(sphericalCoordsRadians.y) * glm::sin(sphericalCoordsRadians.x),
		glm::sin(sphericalCoordsRadians.y),
		glm::cos(sphericalCoordsRadians.y) * glm::cos(sphericalCoordsRadians.x)
	);
	
	_sideOrientation = glm::normalize(glm::cross(glm::vec3(0, 1, 0), _orientation));
	
	_orientationChanged = false;
}

void Camera::recalculateView()
{
	_view = glm::lookAt(getPosition(), getPosition() + getOrientation(), glm::vec3(0, 1, 0));
	
	_viewChanged = false;
}


void Camera::recalculateProjection()
{
	_projection = glm::perspective(_verticalFov, _aspectRatio, NEAR_DISTANCE, FAR_DISTANCE);
	
	_projectionChanged = false;
}

float Camera::getAspectRatio() const
{
	return _aspectRatio;
}

void Camera::setAspectRatio(float aspectRatio)
{
	_aspectRatio = aspectRatio;
	projectionChanged();
}

const std::array<glm::vec3, 4>& Camera::getCornerRays()
{
	if (_cornerRaysChanged) recalculateCornerRays();
	return _cornerRays;
}

void Camera::viewChanged()
{
	_viewChanged = true;
	raysChanged();
}

void Camera::projectionChanged()
{
	_projectionChanged = true;
	raysChanged();
}

void Camera::raysChanged()
{
	_cornerRaysChanged = true;
}

void Camera::recalculateCornerRays()
{
	glm::mat4 vpInverse = glm::inverse(getProjection() * getView());
	
	_cornerRays[0] = glm::vec3(-1, 1, 1); // top left
	_cornerRays[1] = glm::vec3(1, 1, 1); // top right
	_cornerRays[2] = glm::vec3(-1, -1, 1); // bottom left
	_cornerRays[3] = glm::vec3(1, -1, 1); // bottom right
	
	for (int i = 0; i < 4; i++)
	{
		glm::vec4 vec = vpInverse * glm::vec4(_cornerRays[i], 1);
		_cornerRays[i] = glm::normalize(glm::vec3(vec) / vec.w);
	}
}