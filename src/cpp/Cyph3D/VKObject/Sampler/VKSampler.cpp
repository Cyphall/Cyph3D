#include "VKSampler.h"

#include "Cyph3D/VKObject/VKContext.h"

std::shared_ptr<VKSampler> VKSampler::create(VKContext& context, const vk::SamplerCreateInfo& samplerCreateInfo)
{
	return std::shared_ptr<VKSampler>(new VKSampler(context, samplerCreateInfo));
}

VKSampler::VKSampler(VKContext& context, const vk::SamplerCreateInfo& samplerCreateInfo):
	VKObject(context)
{
	_sampler = _context.getDevice().createSampler(samplerCreateInfo);
}

VKSampler::~VKSampler()
{
	_context.getDevice().destroySampler(_sampler);
}

const vk::Sampler& VKSampler::getHandle()
{
	return _sampler;
}