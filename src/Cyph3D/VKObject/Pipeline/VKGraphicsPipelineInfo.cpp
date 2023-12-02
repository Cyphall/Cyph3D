#include "VKGraphicsPipelineInfo.h"

VKGraphicsPipelineInfo::VKGraphicsPipelineInfo(
	const VKPtr<VKPipelineLayout>& pipelineLayout,
	const std::filesystem::path& vertexShader,
	vk::PrimitiveTopology primitiveTopology,
	vk::CullModeFlags cullMode,
	vk::FrontFace frontFace
):
	_pipelineLayout(pipelineLayout),
	_vertexShader(vertexShader),
	_primitiveTopology(primitiveTopology),
	_cullMode(cullMode),
	_frontFace(frontFace)
{
}

const VKPtr<VKPipelineLayout>& VKGraphicsPipelineInfo::getPipelineLayout() const
{
	return _pipelineLayout;
}

const std::filesystem::path& VKGraphicsPipelineInfo::getVertexShader() const
{
	return _vertexShader;
}

const vk::PrimitiveTopology& VKGraphicsPipelineInfo::getPrimitiveTopology() const
{
	return _primitiveTopology;
}

const vk::CullModeFlags& VKGraphicsPipelineInfo::getCullMode() const
{
	return _cullMode;
}

const vk::FrontFace& VKGraphicsPipelineInfo::getFrontFace() const
{
	return _frontFace;
}

VKPipelineVertexInputLayoutInfo& VKGraphicsPipelineInfo::getVertexInputLayoutInfo()
{
	return _vertexInputLayoutInfo;
}

const VKPipelineVertexInputLayoutInfo& VKGraphicsPipelineInfo::getVertexInputLayoutInfo() const
{
	return _vertexInputLayoutInfo;
}

VKPipelineAttachmentInfo& VKGraphicsPipelineInfo::getPipelineAttachmentInfo()
{
	return _pipelineAttachmentInfo;
}

const VKPipelineAttachmentInfo& VKGraphicsPipelineInfo::getPipelineAttachmentInfo() const
{
	return _pipelineAttachmentInfo;
}

bool VKGraphicsPipelineInfo::hasGeometryShader() const
{
	return _geometryShader.has_value();
}

const std::filesystem::path& VKGraphicsPipelineInfo::getGeometryShader() const
{
	return _geometryShader.value();
}

void VKGraphicsPipelineInfo::setGeometryShader(const std::filesystem::path& path)
{
	_geometryShader = path;
}

bool VKGraphicsPipelineInfo::hasFragmentShader() const
{
	return _fragmentShader.has_value();
}

const std::filesystem::path& VKGraphicsPipelineInfo::getFragmentShader() const
{
	return _fragmentShader.value();
}

void VKGraphicsPipelineInfo::setFragmentShader(const std::filesystem::path& path)
{
	_fragmentShader = path;
}

bool VKGraphicsPipelineInfo::hasStaticViewport() const
{
	return _staticViewport.has_value();
}

const VKPipelineViewport& VKGraphicsPipelineInfo::getStaticViewport() const
{
	return _staticViewport.value();
}

void VKGraphicsPipelineInfo::setStaticViewport(const VKPipelineViewport& staticViewport)
{
	_staticViewport = staticViewport;
}

bool VKGraphicsPipelineInfo::hasStaticScissor() const
{
	return _staticScissor.has_value();
}

const VKPipelineScissor& VKGraphicsPipelineInfo::getStaticScissor() const
{
	return _staticScissor.value();
}

void VKGraphicsPipelineInfo::setStaticScissor(const VKPipelineScissor& staticScissor)
{
	_staticScissor = staticScissor;
}

void VKGraphicsPipelineInfo::setRasterizationSampleCount(vk::SampleCountFlagBits sampleCount)
{
	_rasterizationSampleCount = sampleCount;
}

const vk::SampleCountFlagBits& VKGraphicsPipelineInfo::getRasterizationSampleCount() const
{
	return _rasterizationSampleCount;
}