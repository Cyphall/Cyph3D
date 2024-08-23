#pragma once

#include "Cyph3D/VKObject/Pipeline/VKPipelineAttachmentInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineScissor.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineVertexInputLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineViewport.h"

#include <string>
#include <optional>
#include <vulkan/vulkan.hpp>

class VKPipelineLayout;

class VKGraphicsPipelineInfo
{
public:
	VKGraphicsPipelineInfo(
		const std::shared_ptr<VKPipelineLayout>& pipelineLayout,
		const std::string& vertexShader,
		vk::PrimitiveTopology primitiveTopology,
		vk::CullModeFlags cullMode,
		vk::FrontFace frontFace
	);

	const std::shared_ptr<VKPipelineLayout>& getPipelineLayout() const;

	const std::string& getVertexShader() const;

	const vk::PrimitiveTopology& getPrimitiveTopology() const;

	const vk::CullModeFlags& getCullMode() const;

	const vk::FrontFace& getFrontFace() const;

	VKPipelineVertexInputLayoutInfo& getVertexInputLayoutInfo();
	const VKPipelineVertexInputLayoutInfo& getVertexInputLayoutInfo() const;

	VKPipelineAttachmentInfo& getPipelineAttachmentInfo();
	const VKPipelineAttachmentInfo& getPipelineAttachmentInfo() const;

	bool hasGeometryShader() const;
	const std::string& getGeometryShader() const;
	void setGeometryShader(const std::string& path);

	bool hasFragmentShader() const;
	const std::string& getFragmentShader() const;
	void setFragmentShader(const std::string& path);

	bool hasStaticViewport() const;
	const VKPipelineViewport& getStaticViewport() const;
	void setStaticViewport(const VKPipelineViewport& staticViewport);

	bool hasStaticScissor() const;
	const VKPipelineScissor& getStaticScissor() const;
	void setStaticScissor(const VKPipelineScissor& staticScissor);

	void setRasterizationSampleCount(vk::SampleCountFlagBits sampleCount);
	const vk::SampleCountFlagBits& getRasterizationSampleCount() const;

private:
	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::string _vertexShader;
	vk::PrimitiveTopology _primitiveTopology;
	vk::CullModeFlags _cullMode;
	vk::FrontFace _frontFace;
	VKPipelineVertexInputLayoutInfo _vertexInputLayoutInfo;
	VKPipelineAttachmentInfo _pipelineAttachmentInfo;
	std::optional<std::string> _geometryShader;
	std::optional<std::string> _fragmentShader;
	std::optional<VKPipelineViewport> _staticViewport;
	std::optional<VKPipelineScissor> _staticScissor;
	vk::SampleCountFlagBits _rasterizationSampleCount = vk::SampleCountFlagBits::e1;
};