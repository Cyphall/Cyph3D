#pragma once

#include "Resource.h"
#include "../GLObject/Cubemap.h"
#include "StbImage.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <optional>
#include <array>

struct SkyboxLoadingData
{
	GLenum internalFormat;
	std::array<StbImage, 6> data;
	std::array<GLint, 4> swizzle;
};

class Skybox : public Resource<Cubemap, SkyboxLoadingData>
{
public:
	using Resource::Resource;
	Skybox(const Skybox& other) = delete;
	
	float getRotation() const;
	void setRotation(float rotation);

private:
	float _rotation = 0;
	void finishLoading(const SkyboxLoadingData& data) override;
	
	static SkyboxLoadingData loadFromFile(const std::string& name);
	
	friend class ResourceManager;
};


