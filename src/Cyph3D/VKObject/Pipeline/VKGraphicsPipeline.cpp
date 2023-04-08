#include "VKGraphicsPipeline.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Shader/VKShader.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"

VKPtr<VKGraphicsPipeline> VKGraphicsPipeline::create(VKContext& context, VKGraphicsPipelineInfo& info)
{
	return VKPtr<VKGraphicsPipeline>(new VKGraphicsPipeline(context, info));
}

VKDynamic<VKGraphicsPipeline> VKGraphicsPipeline::createDynamic(VKContext& context, VKGraphicsPipelineInfo& info)
{
	return VKDynamic<VKGraphicsPipeline>(context, info);
}

VKGraphicsPipeline::VKGraphicsPipeline(VKContext& context, VKGraphicsPipelineInfo& info):
	VKPipeline(context, info.pipelineLayout)
{
	createPipeline(info);
}

VKGraphicsPipeline::~VKGraphicsPipeline()
{
	_context.getDevice().destroyPipeline(_pipeline);
}

void VKGraphicsPipeline::createPipeline(VKGraphicsPipelineInfo& info)
{
	std::vector<VKPtr<VKShader>> shaders;
	std::vector<vk::PipelineShaderStageCreateInfo> shadersCreateInfos;
	
	if (info.vertexShaderFile.has_value())
	{
		VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, *info.vertexShaderFile));
		
		vk::PipelineShaderStageCreateInfo& createInfo = shadersCreateInfos.emplace_back();
		createInfo.stage = vk::ShaderStageFlagBits::eVertex;
		createInfo.module = shader->getHandle();
		createInfo.pName = "main";
	}
	
	if (info.geometryShaderFile.has_value())
	{
		VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, *info.geometryShaderFile));
		
		vk::PipelineShaderStageCreateInfo& createInfo = shadersCreateInfos.emplace_back();
		createInfo.stage = vk::ShaderStageFlagBits::eGeometry;
		createInfo.module = shader->getHandle();
		createInfo.pName = "main";
	}
	
	if (info.fragmentShaderFile.has_value())
	{
		VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, *info.fragmentShaderFile));
		
		vk::PipelineShaderStageCreateInfo& createInfo = shadersCreateInfos.emplace_back();
		createInfo.stage = vk::ShaderStageFlagBits::eFragment;
		createInfo.module = shader->getHandle();
		createInfo.pName = "main";
	}
	
	vk::PipelineVertexInputStateCreateInfo vertexInputState = info.vertexInputLayoutInfo.get();
	
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.topology = info.vertexTopology;
	inputAssembly.primitiveRestartEnable = false;
	
	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	
	vk::Viewport vkViewport;
	if (info.viewport.has_value())
	{
		vkViewport.x = static_cast<float>(info.viewport->offset.x);
		vkViewport.y = static_cast<float>(info.viewport->offset.y);
		vkViewport.width = static_cast<float>(info.viewport->size.x);
		vkViewport.height = static_cast<float>(info.viewport->size.y);
		vkViewport.minDepth = info.viewport->depthRange.x;
		vkViewport.maxDepth = info.viewport->depthRange.y;
		
		viewportState.pViewports = &vkViewport;
	}
	else
	{
		viewportState.pViewports = nullptr;
	}
	
	vk::Rect2D scissor;
	if (info.scissor.has_value())
	{
		scissor.offset.x = info.scissor->offset.x;
		scissor.offset.y = info.scissor->offset.y;
		scissor.extent.width = info.scissor->size.x;
		scissor.extent.height = info.scissor->size.y;
		
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
	rasterizer.cullMode = info.rasterizationInfo.cullMode;
	rasterizer.frontFace = info.rasterizationInfo.frontFace;
	rasterizer.depthBiasEnable = false;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	
	vk::PipelineMultisampleStateCreateInfo multisampling;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
	multisampling.sampleShadingEnable = false;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = false; // Optional
	multisampling.alphaToOneEnable = false; // Optional
	
	std::vector<vk::Format> colorAttachmentsFormat;
	std::vector<vk::PipelineColorBlendAttachmentState> colorAttachmentsBlending;
	
	colorAttachmentsFormat.reserve(info.pipelineAttachmentInfo._colorAttachmentsInfo.size());
	colorAttachmentsBlending.reserve(info.pipelineAttachmentInfo._colorAttachmentsInfo.size());
	
	for (VKPipelineAttachmentInfo::ColorAttachmentInfo& colorAttachmentInfo : info.pipelineAttachmentInfo._colorAttachmentsInfo)
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
	pipelineRenderingCreateInfo.depthAttachmentFormat = info.pipelineAttachmentInfo._depthAttachmentInfo.format;
	pipelineRenderingCreateInfo.stencilAttachmentFormat = vk::Format::eUndefined;
	
	vk::PipelineDepthStencilStateCreateInfo depthState;
	depthState.depthTestEnable = info.pipelineAttachmentInfo._depthAttachmentInfo.format != vk::Format::eUndefined;
	depthState.depthWriteEnable = info.pipelineAttachmentInfo._depthAttachmentInfo.writeEnabled;
	depthState.depthCompareOp = info.pipelineAttachmentInfo._depthAttachmentInfo.depthTestPassCondition;
	depthState.depthBoundsTestEnable = false;
	depthState.minDepthBounds = 0.0f; // Optional
	depthState.maxDepthBounds = 1.0f; // Optional
	depthState.stencilTestEnable = false;
	depthState.front = vk::StencilOpState(); // Optional
	depthState.back = vk::StencilOpState(); // Optional
	
	std::vector<vk::DynamicState> dynamicStates;
	if (!info.viewport.has_value())
	{
		dynamicStates.push_back(vk::DynamicState::eViewport);
	}
	if (!info.scissor.has_value())
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
	pipelineCreateInfo.layout = _pipelineLayout->getHandle();
	pipelineCreateInfo.renderPass = nullptr;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineCreateInfo.basePipelineIndex = -1; // Optional
	pipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
	
	_pipeline = _context.getDevice().createGraphicsPipeline(VK_NULL_HANDLE, pipelineCreateInfo).value;
}

vk::PipelineBindPoint VKGraphicsPipeline::getPipelineType()
{
	return vk::PipelineBindPoint::eGraphics;
}