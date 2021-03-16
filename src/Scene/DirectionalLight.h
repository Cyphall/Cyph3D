#pragma once

#include "../GLObject/Cubemap.h"
#include "../GLObject/Framebuffer.h"
#include "../GLObject/ShaderProgram.h"
#include "../stdfloat.h"
#include "Light.h"
#include "../GLObject/VertexArray.h"

class DirectionalLight : public Light
{
public:
	struct LightData
	{
		glm::vec3  fragToLightDirection;
		float32_t  intensity;
		glm::vec3  color;
		int32_t    castShadows; // 32-bit bool
		glm::mat4  lightViewProjection;
		uint64_t   shadowMap;
		float32_t  mapSize;
		float32_t  mapDepth;
	};
	
	DirectionalLight(Transform* parent, const std::string& name, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 srgbColor = glm::vec3(1), float intensity = 1, bool castShadows = false);
	DirectionalLight(Transform* parent, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec3 srgbColor = glm::vec3(1), float intensity = 1, bool castShadows = false);
	
	void updateShadowMap(VertexArray& vao);
	
	bool getCastShadows() const;
	void setCastShadows(bool value);
	
	LightData getDataStruct();
	
	int getResolution() const;
	void setResolution(int value);

private:
	std::unique_ptr<Texture> _shadowMap;
	std::unique_ptr<Framebuffer> _shadowMapFb;
	ShaderProgram* _shadowMapProgram;
	
	glm::mat4 _viewProjection;
	static glm::mat4 _projection;
	
	float _mapSize = 60;
	float _mapDepth = 100;
	
	bool _castShadows = false;
	int _resolution = 4096;
	glm::vec3 getLightDirection();
};


