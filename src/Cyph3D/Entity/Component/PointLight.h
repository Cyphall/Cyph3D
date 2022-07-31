#pragma once

#include "Cyph3D/Entity/Component/LightBase.h"

#include <memory>

class Cubemap;
class Framebuffer;

class PointLight : public LightBase
{
public:
	struct RenderData
	{
		glm::vec3     pos;
		float         intensity;
		glm::vec3     color;
		float         radius;
		bool          castShadows; // bool
		glm::mat4     viewProjections[6];
		Cubemap*      shadowMapTexture;
		Framebuffer*  shadowMapFramebuffer;
		int           mapResolution;
		float         far;
	};
	
	explicit PointLight(Entity& entity);
	
	void onPreRender(RenderContext& context) override;
	void onDrawUi() override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	void setCastShadows(bool value);
	bool getCastShadows() const;
	
	void setResolution(int value);
	int getResolution() const;
	
	float getRadius() const;
	void setRadius(float value);
	
	void duplicate(Entity& targetEntity) const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;

private:
	static constexpr float NEAR_DISTANCE = 0.01f;
	static constexpr float FAR_DISTANCE = 100.0f;
	
	std::unique_ptr<Cubemap> _shadowMap;
	std::unique_ptr<Framebuffer> _shadowMapFb;
	
	static glm::mat4 _projection;
	
	float _radius = 0.1f;
	
	bool _castShadows = false;
	int _resolution = 1024;
};