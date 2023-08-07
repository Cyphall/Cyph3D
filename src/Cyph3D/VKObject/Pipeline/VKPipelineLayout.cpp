#include "VKPipelineLayout.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"

VKPtr<VKPipelineLayout> VKPipelineLayout::create(VKContext& context, const VKPipelineLayoutInfo& info)
{
	return VKPtr<VKPipelineLayout>(new VKPipelineLayout(context, info));
}

VKPipelineLayout::VKPipelineLayout(VKContext& context, const VKPipelineLayoutInfo& info):
	VKObject(context),
	_info(info)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.reserve(_info.getDescriptorSetLayouts().size());
	for (const VKPtr<VKDescriptorSetLayout>& descriptorSetLayout : _info.getDescriptorSetLayouts())
	{
		descriptorSetLayouts.emplace_back(descriptorSetLayout->getHandle());
	}
	
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	
	vk::PushConstantRange pushConstantRange;
	if (_info.getPushConstantInfo())
	{
		if (_info.getPushConstantInfo()->size > 128)
		{
			throw;
		}
		
		pushConstantRange.stageFlags = _info.getPushConstantInfo()->shaderStages;
		pushConstantRange.size = _info.getPushConstantInfo()->size;
		pushConstantRange.offset = 0;
		
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
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

const VKPipelineLayoutInfo& VKPipelineLayout::getInfo() const
{
	return _info;
}

const vk::PipelineLayout& VKPipelineLayout::getHandle()
{
	return _pipelineLayout;
}