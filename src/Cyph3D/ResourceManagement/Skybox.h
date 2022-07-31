#pragma once

#include "Cyph3D/GLObject/Cubemap.h"
#include "Cyph3D/ResourceManagement/Resource.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <array>
#include <optional>

class Skybox : public Resource<Cubemap>
{
public:
	using Resource::Resource;
	Skybox(const Skybox& other) = delete;
	
	float getRotation() const;
	void setRotation(float rotation);

private:
	float _rotation = 0;
	
	void loadResourceImpl() override;
};