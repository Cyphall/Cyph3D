#pragma once

#include "Cyph3D/GLObject/BufferBase.h"

struct SamplerCreateInfo;

class Sampler : public BufferBase
{
public:
	explicit Sampler(const SamplerCreateInfo& settings);
	Sampler(const Sampler& other) = delete;
	Sampler(Sampler&& other) = delete;
	
	~Sampler() override;
};