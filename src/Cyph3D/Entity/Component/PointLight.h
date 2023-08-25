#pragma once

#include "Cyph3D/Entity/Component/LightBase.h"
#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"

#include <memory>
#include <nlohmann/json_fwd.hpp>

class VKImage;
class VKImageView;

class PointLight : public LightBase
{
public:
	struct RenderData
	{
		glm::vec3               pos;
		float                   intensity;
		glm::vec3               color;
		float                   radius;
		bool                    castShadows;
		glm::mat4               viewProjections[6];
		VKPtr<VKImageView>*     shadowMapTextureView;
		int                     shadowMapResolution;
		float                   far;
	};
	
	explicit PointLight(Entity& entity);
	
	void onPreRender(RenderRegistry& renderRegistry, Camera& camera) override;
	void onDrawUi() override;
	
	static const char* const identifier;
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
	
	static const vk::Format depthFormat;

private:
	VKPtr<VKImage> _shadowMap;
	VKPtr<VKImageView> _shadowMapView;
	
	glm::mat4 _projection;
	
	float _radius = 0.1f;
	
	bool _castShadows = false;
	int _resolution = 1024;
	
	void deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot);
};