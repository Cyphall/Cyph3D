#pragma once

#include "../GLObject/Cubemap.h"
#include "../GLObject/Framebuffer.h"
#include "../GLObject/ShaderProgram.h"
#include "../stdfloat.h"
#include "Light.h"

class PointLight : public Light
{
public:
	struct LightData
	{
		glm::vec3  pos;
		float32_t  intensity;
		glm::vec3  color;
		int32_t    castShadows; // bool
		uint64_t   shadowMap;
		float32_t  _far;
		float32_t  padding;
	};
	
	PointLight(Transform* parent, const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 srgbColor = glm::vec3(1), float intensity = 1, bool castShadows = false);
	PointLight(Transform* parent, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec3 srgbColor = glm::vec3(1), float intensity = 1, bool castShadows = false);
	
	void updateShadowMap();
	
	void setCastShadows(bool value);
	bool getCastShadows() const;
	LightData getDataStruct();
	
private:
	static constexpr int _RESOLUTION = 1024;
	static constexpr float _NEAR = 0.01f;
	static constexpr float _FAR = 100.0f;
	
	std::unique_ptr<Cubemap> _shadowMap;
	std::unique_ptr<Framebuffer> _shadowMapFb;
	ShaderProgram* _shadowMapProgram;
	
	glm::mat4 _viewProjections[6];
	static glm::mat4 _projection;
	
	bool _castShadows = false;
};
