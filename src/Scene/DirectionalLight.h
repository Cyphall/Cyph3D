#pragma once

#include "../GLObject/Cubemap.h"
#include "../GLObject/Framebuffer.h"
#include "../GLObject/ShaderProgram.h"
#include "../stdfloat.h"
#include "Light.h"

class DirectionalLight : public Light
{
public:
	struct LightData
	{
		glm::vec3  fragToLightDirection;
		float32_t  intensity;
		glm::vec3  color;
		int32_t    castShadows; // bool
		glm::mat4  lightViewProjection;
		uint64_t   shadowMap;
		glm::vec2  padding;
	};
	
	DirectionalLight(Transform* parent, const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 srgbColor = glm::vec3(1), float intensity = 1, bool castShadows = false);
	DirectionalLight(Transform* parent, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec3 srgbColor = glm::vec3(1), float intensity = 1, bool castShadows = false);
	
	void updateShadowMap();
	
	void setCastShadows(bool value);
	bool getCastShadows() const;
	LightData getDataStruct();

private:
	static constexpr int RESOLUTION = 4096;
	
	std::unique_ptr<Texture> _shadowMap;
	std::unique_ptr<Framebuffer> _shadowMapFb;
	ShaderProgram* _shadowMapProgram;
	
	glm::mat4 _viewProjection;
	static glm::mat4 _projection;
	
	bool _castShadows = false;
	
	glm::vec3 getLightDirection();
};


