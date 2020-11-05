#pragma once

#include "Resource.h"
#include "../GLObject/Cubemap.h"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <optional>
#include <array>

struct SkyboxLoadingData
{
	glm::ivec2 size;
	GLenum internalFormat;
	GLenum pixelFormat;
	std::array<std::unique_ptr<uint8_t[]>, 6> data;
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
	float _rotation;
	void finishLoading(const SkyboxLoadingData& data) override;
	
	static SkyboxLoadingData loadFromFile(const std::string& name);
	
	friend class ResourceManager;
};


