#pragma once

#include "SceneObject.h"

class Light : public SceneObject
{
public:
	glm::vec3 getLinearColor() const;
	void setLinearColor(glm::vec3 color);
	
	glm::vec3 getSrgbColor() const;
	void setSrgbColor(glm::vec3 color);
	
	float getIntensity() const;
	void setIntensity(float intensity);

protected:
	Light(Transform* parent, const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 srgbColor, float intensity);
	Light(Transform* parent, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec3 srgbColor, float intensity);
	
	glm::vec3 _linearColor;
	glm::vec3 _srgbColor;
	float _intensity;
	
private:
	static glm::vec3 toLinear(glm::vec3 color);
	static glm::vec3 toSrgb(glm::vec3 color);
};


