#pragma once

#include "Cyph3D/GLObject/GLObject.h"

struct SamplerCreateInfo;

class GLSampler : public GLObject
{
public:
	explicit GLSampler(const SamplerCreateInfo& settings);
	~GLSampler() override;
};