#pragma once

#include "Cyph3D/Entity/Component/LightBase.h"

#include <nlohmann/json_fwd.hpp>

class PointLight : public LightBase
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

	explicit PointLight(Entity& entity);

	void onPreRender(RenderRegistry& renderRegistry, Camera& camera) override;
	void onDrawUi() override;

	static const char* const identifier;
	const char* getIdentifier() const override;

	void setCastShadows(bool value);
	bool getCastShadows() const;

	void setResolution(uint32_t value);
	uint32_t getResolution() const;

	float getRadius() const;
	void setRadius(float value);

	void duplicate(Entity& targetEntity) const override;

	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& serialization) override;

private:
	float _radius = 0.1f;

	bool _castShadows = false;
	uint32_t _resolution = 1024;

	void deserializeFromVersion1(const nlohmann::ordered_json& jsonRoot);
	void deserializeFromVersion2(const nlohmann::ordered_json& jsonRoot);
};