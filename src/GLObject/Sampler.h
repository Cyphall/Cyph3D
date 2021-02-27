#pragma once

#include "BufferBase.h"
#include "CreateInfo/SamplerCreateInfo.h"

class Sampler : public BufferBase
{
public:
	explicit Sampler(const SamplerCreateInfo& settings);
	Sampler(const Sampler& other) = delete;
	Sampler(Sampler&& other) = delete;
	
	~Sampler() override;
};
