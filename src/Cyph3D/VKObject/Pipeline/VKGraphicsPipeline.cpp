#include "VKGraphicsPipeline.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Shader/VKShader.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"

VKPtr<VKGraphicsPipeline> VKGraphicsPipeline::create(VKContext& context, VKGraphicsPipelineInfo& info)
{
	return VKPtr<VKGraphicsPipeline>(new VKGraphicsPipeline(context, info));
}

VKGraphicsPipeline::VKGraphicsPipeline(VKContext& context, VKGraphicsPipelineInfo& info):
	VKPipeline(context), _info(info)
{
	std::vector<VKPtr<VKShader>> shaders;
	std::vector<vk::PipelineShaderStageCreateInfo> shadersCreateInfos;
	
	{
		VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, _info.getVertexShader()));
		
		vk::PipelineShaderStageCreateInfo& createInfo = shadersCreateInfos.emplace_back();
		createInfo.stage = vk::ShaderStageFlagBits::eVertex;
		createInfo.module = shader->getHandle();
		createInfo.pName = "main";
	}
	
	if (_info.hasGeometryShader())
	{
		VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, _info.getGeometryShader()));
		
		vk::PipelineShaderStageCreateInfo& createInfo = shadersCreateInfos.emplace_back();
		createInfo.stage = vk::ShaderStageFlagBits::eGeometry;
		createInfo.module = shader->getHandle();
		createInfo.pName = "main";
	}
	
	if (_info.hasFragmentShader())
	{
		VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, _info.getFragmentShader()));
		
		vk::PipelineShaderStageCreateInfo& createInfo = shadersCreateInfos.emplace_back();
		createInfo.stage = vk::ShaderStageFlagBits::eFragment;
		createInfo.module = shader->getHandle();
		createInfo.pName = "main";
	}
	
	vk::PipelineVertexInputStateCreateInfo vertexInputState = _info.getVertexInputLayoutInfo().get();
	
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = _info.getPrimitiveTopology();
	inputAssembly.primitiveRestartEnable = false;
	
	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	
	vk::Viewport vkViewport;
	if (_info.hasStaticViewport())
	{
		vkViewport.x = _info.getStaticViewport().offset.x;
		vkViewport.y = _info.getStaticViewport().offset.y;
		vkViewport.width = _info.getStaticViewport().size.x;
		vkViewport.height = _info.getStaticViewport().size.y;
		vkViewport.minDepth = _info.getStaticViewport().depthRange.x;
		vkViewport.maxDepth = _info.getStaticViewport().depthRange.y;
		
		viewportState.pViewports = &vkViewport;
	}
	else
	{
		viewportState.pViewports = nullptr;
	}
	
	vk::Rect2D scissor;
	if (_info.hasStaticScissor())
	{
		scissor.offset.x = _info.getStaticScissor().offset.x;
		scissor.offset.y = _info.getStaticScissor().offset.y;
		scissor.extent.width = _info.getStaticScissor().size.x;
		scissor.extent.height = _info.getStaticScissor().size.y;
		
		viewportState.pScissors = &scissor;
	}
	else
	{
		viewportState.pScissors = nullptr;
	}
	
	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.depthClampEnable = false;
	rasterizer.rasterizerDiscardEnable = false;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = _info.getCullMode();
	rasterizer.frontFace = _info.getFrontFace();
	rasterizer.depthBiasEnable = false;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	
	vk::PipelineMultisampleStateCreateInfo multisampling;
	multisampling.rasterizationSamples = _info.getRasterizationSampleCount();
	multisampling.sampleShadingEnable = false;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = false; // Optional
	multisampling.alphaToOneEnable = false; // Optional
	
	std::vector<vk::Format> colorAttachmentsFormat;
	std::vector<vk::PipelineColorBlendAttachmentState> colorAttachmentsBlending;
	
	colorAttachmentsFormat.reserve(_info.getPipelineAttachmentInfo().getColorAttachmentsInfos().size());
	colorAttachmentsBlending.reserve(_info.getPipelineAttachmentInfo().getColorAttachmentsInfos().size());
	
	for (const VKPipelineAttachmentInfo::ColorAttachmentInfo& colorAttachmentInfo : _info.getPipelineAttachmentInfo().getColorAttachmentsInfos())
	{
		colorAttachmentsFormat.emplace_back(colorAttachmentInfo.format);
		
		vk::PipelineColorBlendAttachmentState& blending = colorAttachmentsBlending.emplace_back();
		if (colorAttachmentInfo.blending.has_value())
		{
			blending.blendEnable = true;
			blending.srcColorBlendFactor =  colorAttachmentInfo.blending->srcColorBlendFactor;
			blending.dstColorBlendFactor =  colorAttachmentInfo.blending->dstColorBlendFactor;
			blending.colorBlendOp =  colorAttachmentInfo.blending->colorBlendOp;
			blending.srcAlphaBlendFactor =  colorAttachmentInfo.blending->srcAlphaBlendFactor;
			blending.dstAlphaBlendFactor =  colorAttachmentInfo.blending->dstAlphaBlendFactor;
			blending.alphaBlendOp =  colorAttachmentInfo.blending->alphaBlendOp;
			blending.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		}
		else
		{
			blending.blendEnable = false;
			blending.srcColorBlendFactor = vk::BlendFactor::eOne; // Optional
			blending.dstColorBlendFactor = vk::BlendFactor::eZero; // Optional
			blending.colorBlendOp = vk::BlendOp::eAdd; // Optional
			blending.srcAlphaBlendFactor = vk::BlendFactor::eOne; // Optional
			blending.dstAlphaBlendFactor = vk::BlendFactor::eZero; // Optional
			blending.alphaBlendOp = vk::BlendOp::eAdd; // Optional
			blending.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		}
	}
	
	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.logicOpEnable = false;
	colorBlending.logicOp = vk::LogicOp::eCopy; // Optional
	colorBlending.attachmentCount = colorAttachmentsBlending.size();
	colorBlending.pAttachments = colorAttachmentsBlending.data();
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional
	
	vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo;
	pipelineRenderingCreateInfo.viewMask = 0;
	pipelineRenderingCreateInfo.colorAttachmentCount = colorAttachmentsFormat.size();
	pipelineRenderingCreateInfo.pColorAttachmentFormats = colorAttachmentsFormat.data();
	pipelineRenderingCreateInfo.depthAttachmentFormat = _info.getPipelineAttachmentInfo().hasDepthAttachment() ? _info.getPipelineAttachmentInfo().getDepthAttachmentInfo().format : vk::Format::eUndefined;
	pipelineRenderingCreateInfo.stencilAttachmentFormat = vk::Format::eUndefined;
	
	vk::PipelineDepthStencilStateCreateInfo depthState;
	depthState.depthTestEnable = _info.getPipelineAttachmentInfo().hasDepthAttachment();
	depthState.depthWriteEnable = _info.getPipelineAttachmentInfo().hasDepthAttachment() ? _info.getPipelineAttachmentInfo().getDepthAttachmentInfo().writeEnabled : false;
	depthState.depthCompareOp = _info.getPipelineAttachmentInfo().hasDepthAttachment() ? _info.getPipelineAttachmentInfo().getDepthAttachmentInfo().depthTestPassCondition : vk::CompareOp::eAlways;
	depthState.depthBoundsTestEnable = false;
	depthState.minDepthBounds = 0.0f; // Optional
	depthState.maxDepthBounds = 1.0f; // Optional
	depthState.stencilTestEnable = false;
	depthState.front = vk::StencilOpState(); // Optional
	depthState.back = vk::StencilOpState(); // Optional
	
	std::vector<vk::DynamicState> dynamicStates;
	if (!_info.hasStaticViewport())
	{
		dynamicStates.push_back(vk::DynamicState::eViewport);
	}
	if (!_info.hasStaticScissor())
	{
		dynamicStates.push_back(vk::DynamicState::eScissor);
	}
	
	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
	dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
	
	vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.stageCount = shadersCreateInfos.size();
	pipelineCreateInfo.pStages = shadersCreateInfos.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = &depthState; // Optional
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = _info.getPipelineLayout()->getHandle();
	pipelineCreateInfo.renderPass = nullptr;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineCreateInfo.basePipelineIndex = -1; // Optional
	pipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
	
	_pipeline = _context.getDevice().createGraphicsPipeline(VK_NULL_HANDLE, pipelineCreateInfo).value;
}

VKGraphicsPipeline::~VKGraphicsPipeline()
{
	_context.getDevice().destroyPipeline(_pipeline);
}

const VKGraphicsPipelineInfo& VKGraphicsPipeline::getInfo() const
{
	return _info;
}

vk::PipelineBindPoint VKGraphicsPipeline::getPipelineType() const
{
	return vk::PipelineBindPoint::eGraphics;
}

const VKPtr<VKPipelineLayout>& VKGraphicsPipeline::getPipelineLayout() const
{
	return _info.getPipelineLayout();
}