#pragma once

#include "Cyph3D/GLObject/BufferBase.h"
#include "Cyph3D/GLObject/CreateInfo/SamplerCreateInfo.h"

class Sampler : public BufferBase
{
public:
	explicit Sampler(const SamplerCreateInfo& settings);
	Sampler(const Sampler& other) = delete;
	Sampler(Sampler&& other) = delete;
	
	~Sampler() override;
};