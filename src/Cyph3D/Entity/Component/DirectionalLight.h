#pragma once

#include "Cyph3D/Entity/Component/LightBase.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"

#include <memory>

class VKImage;
class VKImageView;

class DirectionalLight : public LightBase
{
public:
	struct RenderData
	{
		glm::vec3               fragToLightDirection;
		float                   intensity;
		glm::vec3               color;
		float                   angularDiameter;
		bool                    castShadows;
		glm::mat4               lightViewProjection;
		VKDynamic<VKImage>*     shadowMapTexture;
		VKDynamic<VKImageView>* shadowMapTextureView;
		float                   shadowMapTexelWorldSize;
	};

	explicit DirectionalLight(Entity& entity);
	
	void onPreRender(SceneRenderer& sceneRenderer, Camera& camera) override;
	void onDrawUi() override;
	
	static const char* const identifier;
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
	
	static const vk::Format depthFormat;

private:
	VKDynamic<VKImage> _shadowMap;
	VKDynamic<VKImageView> _shadowMapView;
	
	glm::mat4 _projection;
	
	float _mapSize = 60;
	float _mapDepth = 100;
	
	float _angularDiameter = 0.53f;
	
	bool _castShadows = false;
	int _resolution = 4096;
	
	glm::vec3 getLightDirection();
};