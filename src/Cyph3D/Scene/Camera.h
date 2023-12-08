#pragma once

#include <array>
#include <glm/glm.hpp>

class Camera
{
public:
	explicit Camera(glm::vec3 position = glm::vec3(0), glm::vec2 sphericalCoords = glm::vec2(0));

	bool update(glm::vec2 mousePosDelta);

	glm::mat4 getView() const;

	glm::mat4 getProjection() const;

	glm::vec3 getPosition() const;
	void setPosition(glm::vec3 position);

	glm::vec2 getSphericalCoords() const;
	void setSphericalCoords(glm::vec2 sphericalCoords);

	float getSpeed() const;
	void setSpeed(float speed);

	float getAperture() const;
	void setAperture(float aperture);
	
	glm::vec2 getShutterSpeed() const;
	void setShutterSpeed(glm::vec2 shutterSpeed);
	
	float getSensitivity() const;
	void setSensitivity(float sensitivity);
	
	float calcExposure() const;

	float getVerticalFov() const;
	void setVerticalFov(float vfov);
	void setHorizontalFov(float hfov, float referenceAspectRatio);

	float getAspectRatio() const;
	void setAspectRatio(float aspectRatio);

	const std::array<glm::vec3, 4>& getCornerRays() const;

private:
	void orientationChanged() const;
	mutable bool _orientationChanged = true;
	mutable glm::vec3 _orientation = glm::vec3(0);
	mutable glm::vec3 _sideOrientation = glm::vec3(0);

	void viewChanged() const;
	mutable bool _viewChanged = true;
	mutable glm::mat4 _view = glm::mat4(0);

	void projectionChanged() const;
	mutable bool _projectionChanged = true;
	mutable glm::mat4 _projection = glm::mat4(0);

	glm::vec3 _position;

	// x: phi (horizontal) 0 to 360
	// y: theta (vertical) -89 to 89
	glm::vec2 _sphericalCoords;

	float _speed = 2;
	
	float _aperture = 16.0f;
	glm::vec2 _shutterSpeed = {1, 125};
	float _sensitivity = 100.0f;

	float _verticalFov;
	float _aspectRatio;

	void cornerRaysChanged() const;
	mutable bool _cornerRaysChanged = true;
	mutable std::array<glm::vec3, 4> _cornerRays;

	glm::vec3 getOrientation() const;
	glm::vec3 getSideOrientation() const;

	void recalculateOrientation() const;
	void recalculateView() const;
	void recalculateProjection() const;
	void recalculateCornerRays() const;

	static constexpr float NEAR_DISTANCE = 0.02f;
	static constexpr float FAR_DISTANCE = 1000.0f;
};