#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	Camera(glm::vec3 position = glm::vec3(0), glm::vec2 sphericalCoords = glm::vec2(0));
	
	void update(double deltaTime);
	
	glm::vec3 getOrientation();
	
	glm::vec3 getSideOrientation();
	
	glm::mat4 getView();
	
	glm::mat4 getProjection();
	
	glm::vec3 getPosition() const;
	void setPosition(glm::vec3 position);
	__declspec(property(get = getPosition, put = setPosition)) glm::vec3 position;
	
	glm::vec2 getSphericalCoords() const;
	void setSphericalCoords(glm::vec2 sphericalCoords);
	__declspec(property(get = getSphericalCoords, put = setSphericalCoords)) glm::vec2 sphericalCoords;
	
	float getSpeed() const;
	void setSpeed(float speed);
	__declspec(property(get = getSpeed, put = setSpeed)) float speed;
	
	float getExposure() const;
	void setExposure(float exposure);
	__declspec(property(get = getExposure, put = setExposure)) float exposure;
	
	float getFov() const;
	void setFov(float fov);
	__declspec(property(get = getFov, put = setFov)) float fov;

private:
	bool _orientationChanged = true;
	glm::vec3 _orientation = glm::vec3(0);
	glm::vec3 _sideOrientation = glm::vec3(0);
	
	bool _viewChanged = true;
	glm::mat4 _view = glm::mat4(0);
	
	bool _projectionChanged = true;
	glm::mat4 _projection = glm::mat4(0);
	
	glm::vec3 _position;
	
	// x: phi (horizontal) 0 to 360
	// y: theta (vertical) -89 to 89
	glm::vec2 _sphericalCoords;
	
	glm::vec2 _previousMousePos;
	
	float _speed = 2;
	float _exposure = 1;
	float _fov;
	
	void recalculateOrientation();
	void recalculateView();
	void recalculateProjection();
};
