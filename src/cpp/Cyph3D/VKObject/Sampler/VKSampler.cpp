#include "VKSampler.h"

#include "Cyph3D/VKObject/VKContext.h"

std::shared_ptr<c3d::VKSampler> c3d::VKSampler::create(VKContext& context, const vk::SamplerCreateInfo& samplerCreateInfo)
{
	return std::shared_ptr<VKSampler>(new VKSampler(context, samplerCreateInfo));
}

c3d::VKSampler::VKSampler(VKContext& context, const vk::SamplerCreateInfo& samplerCreateInfo):
	VKObject(context)
{
	_sampler = _context.getDevice().createSampler(samplerCreateInfo);
}

c3d::VKSampler::~VKSampler()
{
	_context.getDevice().destroySampler(_sampler);
}

const vk::Sampler& c3d::VKSampler::getHandle()
{
	return _sampler;
}