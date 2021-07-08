#pragma once

#include <glm/glm.hpp>
#include <array>

class Camera
{
public:
	Camera(glm::vec3 position = glm::vec3(0), glm::vec2 sphericalCoords = glm::vec2(0));
	
	void update(glm::vec2 mousePos);
	
	glm::mat4 getView();
	
	glm::mat4 getProjection();
	
	glm::vec3 getPosition() const;
	void setPosition(glm::vec3 position);
	
	glm::vec2 getSphericalCoords() const;
	void setSphericalCoords(glm::vec2 sphericalCoords);
	
	float getSpeed() const;
	void setSpeed(float speed);
	
	float getExposure() const;
	void setExposure(float exposure);
	
	float getVerticalFov() const;
	void setVerticalFov(float vfov);
	void setHorizontalFov(float hfov, float referenceAspectRatio);
	
	float getAspectRatio() const;
	void setAspectRatio(float aspectRatio);
	
	const std::array<glm::vec3, 4>& getCornerRays();

private:
	bool _orientationChanged = true;
	glm::vec3 _orientation = glm::vec3(0);
	glm::vec3 _sideOrientation = glm::vec3(0);
	
	void viewChanged();
	bool _viewChanged = true;
	glm::mat4 _view = glm::mat4(0);
	
	void projectionChanged();
	bool _projectionChanged = true;
	glm::mat4 _projection = glm::mat4(0);
	
	glm::vec3 _position;
	
	// x: phi (horizontal) 0 to 360
	// y: theta (vertical) -89 to 89
	glm::vec2 _sphericalCoords;
	
	float _speed = 2;
	float _exposure = 0;
	
	float _verticalFov;
	float _aspectRatio;
	
	void raysChanged();
	bool _cornerRaysChanged;
	std::array<glm::vec3, 4> _cornerRays;
	
	glm::vec3 getOrientation();
	glm::vec3 getSideOrientation();
	
	void recalculateOrientation();
	void recalculateView();
	void recalculateProjection();
	void recalculateCornerRays();
	
	static float NEAR_DISTANCE;
	static float FAR_DISTANCE;
};