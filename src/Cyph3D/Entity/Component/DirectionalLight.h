#pragma once

#include "Cyph3D/Entity/Component/LightBase.h"

#include <memory>

class GLTexture;
class GLFramebuffer;

class DirectionalLight : public LightBase
{
public:
	struct RenderData
	{
		glm::vec3      fragToLightDirection;
		float          intensity;
		glm::vec3      color;
		float          angularDiameter;
		bool           castShadows; // 32-bit bool
		glm::mat4      lightViewProjection;
		GLTexture*       shadowMapTexture;
		GLFramebuffer*   shadowMapFramebuffer;
		int            mapResolution;
		float          mapSize;
		float          mapDepth;
	};

	explicit DirectionalLight(Entity& entity);
	
	void onPreRender(SceneRenderer& sceneRenderer, Camera& camera) override;
	void onDrawUi() override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	bool getCastShadows() const;
	void setCastShadows(bool value);
	
	int getResolution() const;
	void setResolution(int value);
	
	float getAngularDiameter() const;
	void setAngularDiameter(float value);
	
	void duplicate(Entity& targetEntity) const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;

private:
	std::unique_ptr<GLTexture> _shadowMap;
	std::unique_ptr<GLFramebuffer> _shadowMapFb;
	
	static glm::mat4 _projection;
	
	float _mapSize = 60;
	float _mapDepth = 100;
	
	float _angularDiameter = 0.53f;
	
	bool _castShadows = false;
	int _resolution = 4096;
	
	glm::vec3 getLightDirection();
};