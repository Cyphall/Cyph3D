#pragma once

#include "Resource.h"
#include "../GLObject/Cubemap.h"
#include "StbImage.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <optional>
#include <array>

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


