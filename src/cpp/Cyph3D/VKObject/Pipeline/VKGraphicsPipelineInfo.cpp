#include "VKGraphicsPipelineInfo.h"

c3d::VKGraphicsPipelineInfo::VKGraphicsPipelineInfo(
	const std::shared_ptr<VKPipelineLayout>& pipelineLayout,
	const std::string& vertexShader,
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

const std::shared_ptr<c3d::VKPipelineLayout>& c3d::VKGraphicsPipelineInfo::getPipelineLayout() const
{
	return _pipelineLayout;
}

const std::string& c3d::VKGraphicsPipelineInfo::getVertexShader() const
{
	return _vertexShader;
}

const vk::PrimitiveTopology& c3d::VKGraphicsPipelineInfo::getPrimitiveTopology() const
{
	return _primitiveTopology;
}

const vk::CullModeFlags& c3d::VKGraphicsPipelineInfo::getCullMode() const
{
	return _cullMode;
}

const vk::FrontFace& c3d::VKGraphicsPipelineInfo::getFrontFace() const
{
	return _frontFace;
}

c3d::VKPipelineVertexInputLayoutInfo& c3d::VKGraphicsPipelineInfo::getVertexInputLayoutInfo()
{
	return _vertexInputLayoutInfo;
}

const c3d::VKPipelineVertexInputLayoutInfo& c3d::VKGraphicsPipelineInfo::getVertexInputLayoutInfo() const
{
	return _vertexInputLayoutInfo;
}

c3d::VKPipelineAttachmentInfo& c3d::VKGraphicsPipelineInfo::getPipelineAttachmentInfo()
{
	return _pipelineAttachmentInfo;
}

const c3d::VKPipelineAttachmentInfo& c3d::VKGraphicsPipelineInfo::getPipelineAttachmentInfo() const
{
	return _pipelineAttachmentInfo;
}

bool c3d::VKGraphicsPipelineInfo::hasGeometryShader() const
{
	return _geometryShader.has_value();
}

const std::string& c3d::VKGraphicsPipelineInfo::getGeometryShader() const
{
	return _geometryShader.value();
}

void c3d::VKGraphicsPipelineInfo::setGeometryShader(const std::string& path)
{
	_geometryShader = path;
}

bool c3d::VKGraphicsPipelineInfo::hasFragmentShader() const
{
	return _fragmentShader.has_value();
}

const std::string& c3d::VKGraphicsPipelineInfo::getFragmentShader() const
{
	return _fragmentShader.value();
}

void c3d::VKGraphicsPipelineInfo::setFragmentShader(const std::string& path)
{
	_fragmentShader = path;
}

bool c3d::VKGraphicsPipelineInfo::hasStaticViewport() const
{
	return _staticViewport.has_value();
}

const c3d::VKPipelineViewport& c3d::VKGraphicsPipelineInfo::getStaticViewport() const
{
	return _staticViewport.value();
}

void c3d::VKGraphicsPipelineInfo::setStaticViewport(const VKPipelineViewport& staticViewport)
{
	_staticViewport = staticViewport;
}

bool c3d::VKGraphicsPipelineInfo::hasStaticScissor() const
{
	return _staticScissor.has_value();
}

const c3d::VKPipelineScissor& c3d::VKGraphicsPipelineInfo::getStaticScissor() const
{
	return _staticScissor.value();
}

void c3d::VKGraphicsPipelineInfo::setStaticScissor(const VKPipelineScissor& staticScissor)
{
	_staticScissor = staticScissor;
}

void c3d::VKGraphicsPipelineInfo::setRasterizationSampleCount(vk::SampleCountFlagBits sampleCount)
{
	_rasterizationSampleCount = sampleCount;
}

const vk::SampleCountFlagBits& c3d::VKGraphicsPipelineInfo::getRasterizationSampleCount() const
{
	return _rasterizationSampleCount;
}