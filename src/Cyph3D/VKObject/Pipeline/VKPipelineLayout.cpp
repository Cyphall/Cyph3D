#include "VKPipelineLayout.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"

VKPtr<VKPipelineLayout> VKPipelineLayout::create(VKContext& context, const VKPipelineLayoutInfo& pipelineLayoutInfo)
{
	return VKPtr<VKPipelineLayout>(new VKPipelineLayout(context, pipelineLayoutInfo));
}

VKDynamic<VKPipelineLayout> VKPipelineLayout::createDynamic(VKContext& context, const VKPipelineLayoutInfo& pipelineLayoutInfo)
{
	return VKDynamic<VKPipelineLayout>(context, pipelineLayoutInfo);
}

VKPipelineLayout::VKPipelineLayout(VKContext& context, const VKPipelineLayoutInfo& pipelineLayoutInfo):
	VKObject(context)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.setLayoutCount = pipelineLayoutInfo._vkDescriptorSetsLayouts.size();
	pipelineLayoutCreateInfo.pSetLayouts = pipelineLayoutInfo._vkDescriptorSetsLayouts.data();
	
	_descriptorSetsLayouts = pipelineLayoutInfo._descriptorSetsLayouts;
	
	if (pipelineLayoutInfo._pushConstantRange.has_value())
	{
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pipelineLayoutInfo._pushConstantRange.value();
	}
	else
	{
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	}
	
	_pipelineLayout = _context.getDevice().createPipelineLayout(pipelineLayoutCreateInfo);
}

VKPipelineLayout::~VKPipelineLayout()
{
	_context.getDevice().destroyPipelineLayout(_pipelineLayout);
}

const vk::PipelineLayout& VKPipelineLayout::getHandle()
{
	return _pipelineLayout;
}

const VKPtr<VKDescriptorSetLayout>& VKPipelineLayout::getDescriptorSetLayout(uint32_t setIndex)
{
	return _descriptorSetsLayouts.at(setIndex);
}