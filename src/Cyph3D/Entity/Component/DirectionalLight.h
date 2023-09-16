#pragma once

#include "Cyph3D/Entity/Component/LightBase.h"

#include <nlohmann/json_fwd.hpp>
#include <memory>

class DirectionalLight : public LightBase
{
public:
	struct RenderData
	{
		Transform&          transform;
		float               intensity;
		glm::vec3           color;
		bool                castShadows;
		uint32_t            shadowMapResolution;
	};

	explicit DirectionalLight(Entity& entity);
	
	void onPreRender(RenderRegistry& renderRegistry, Camera& camera) override;
	void onDrawUi() override;
	
	static const char* const identifier;
	const char* getIdentifier() const override;
	
	bool getCastShadows() const;
	void setCastShadows(bool value);
	
	uint32_t getResolution() const;
	void setResolution(uint32_t value);
	
	float getAngularDiameter() const;
	void setAngularDiameter(float value);
	
	void duplicate(Entity& targetEntity) const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;

private:
	float _angularDiameter = 0.53f;
	
	bool _castShadows = false;
	uint32_t _resolution = 4096;
	
	void deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot);
};