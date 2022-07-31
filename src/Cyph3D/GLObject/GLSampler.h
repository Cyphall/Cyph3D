#pragma once

#include "Cyph3D/GLObject/BufferBase.h"

struct SamplerCreateInfo;

class GLSampler : public BufferBase
{
public:
	explicit GLSampler(const SamplerCreateInfo& settings);
	GLSampler(const GLSampler& other) = delete;
	GLSampler(GLSampler&& other) = delete;
	
	~GLSampler() override;
};