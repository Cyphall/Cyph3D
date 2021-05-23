#pragma once

#include "LightBase.h"
#include "../../stdfloat.h"
#include "../../GLObject/VertexArray.h"
#include "../../GLObject/Cubemap.h"
#include "../../GLObject/Framebuffer.h"

class RenderRegistry;

class PointLight : public LightBase
{
public:
	struct NativeData
	{
		glm::vec3  pos;
		float32_t  intensity;
		glm::vec3  color;
		int32_t    castShadows; // bool
		uint64_t   shadowMap;
		float32_t  _far;
		float32_t  maxTexelSizeAtUnitDistance;
	};
	
	struct RenderData
	{
		NativeData nativeData;
		PointLight* light;
	};
	
	PointLight(Entity& entity);
	
	void updateShadowMap(VertexArray& vao, RenderRegistry& registry);
	
	void onPreRender() override;
	void onDrawUi() override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	void setCastShadows(bool value);
	bool getCastShadows() const;
	
	void setResolution(int value);
	int getResolution() const;
	
	ComponentSerialization serialize() const override;
	void deserialize(const ComponentSerialization& data) override;

private:
	static constexpr float _NEAR = 0.01f;
	static constexpr float _FAR = 100.0f;
	
	std::unique_ptr<Cubemap> _shadowMap;
	std::unique_ptr<Framebuffer> _shadowMapFb;
	ShaderProgram* _shadowMapProgram;
	
	glm::mat4 _viewProjections[6];
	static glm::mat4 _projection;
	
	bool _castShadows = false;
	int _resolution = 1024;
};