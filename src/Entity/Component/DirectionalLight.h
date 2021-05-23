#pragma once

#include "../../GLObject/Cubemap.h"
#include "../../GLObject/Framebuffer.h"
#include "../../GLObject/ShaderProgram.h"
#include "../../stdfloat.h"
#include "LightBase.h"
#include "../../GLObject/VertexArray.h"

class RenderRegistry;

class DirectionalLight : public LightBase
{
public:
	struct NativeData
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
	
	struct RenderData
	{
		NativeData nativeData;
		DirectionalLight* light;
	};

	DirectionalLight(Entity& entity);
	
	void updateShadowMap(VertexArray& vao, RenderRegistry& registry);
	
	void onPreRender() override;
	void onDrawUi() override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	bool getCastShadows() const;
	void setCastShadows(bool value);
	
	int getResolution() const;
	void setResolution(int value);
	
	ComponentSerialization serialize() const override;
	void deserialize(const ComponentSerialization& data) override;

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
