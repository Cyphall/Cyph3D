#pragma once

#include "Cyph3D/GLObject/Cubemap.h"
#include "Cyph3D/GLObject/Framebuffer.h"
#include "Cyph3D/GLObject/ShaderProgram.h"
#include "Cyph3D/Entity/Component/LightBase.h"
#include "Cyph3D/GLObject/VertexArray.h"

struct RenderRegistry;
class Renderer;

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
		Texture*       shadowMapTexture;
		Framebuffer*   shadowMapFramebuffer;
		int            mapResolution;
		float          mapSize;
		float          mapDepth;
	};

	explicit DirectionalLight(Entity& entity);
	
	void onPreRender(RenderContext& context) override;
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
	std::unique_ptr<Texture> _shadowMap;
	std::unique_ptr<Framebuffer> _shadowMapFb;
	
	static glm::mat4 _projection;
	
	float _mapSize = 60;
	float _mapDepth = 100;
	
	float _angularDiameter = 0.53f;
	
	bool _castShadows = false;
	int _resolution = 4096;
	
	glm::vec3 getLightDirection();
};