#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineVertexInputLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineViewport.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineScissor.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineAttachmentInfo.h"

#include <vulkan/vulkan.hpp>
#include <filesystem>
#include <optional>

class VKPipelineLayout;

class VKGraphicsPipelineInfo
{
public:
	VKGraphicsPipelineInfo(
		const VKPtr<VKPipelineLayout>& pipelineLayout,
		const std::filesystem::path& vertexShader,
		vk::PrimitiveTopology primitiveTopology,
		vk::CullModeFlags cullMode,
		vk::FrontFace frontFace);
	
	const VKPtr<VKPipelineLayout>& getPipelineLayout() const;
	
	const std::filesystem::path& getVertexShader() const;
	
	const vk::PrimitiveTopology& getPrimitiveTopology() const;
	
	const vk::CullModeFlags& getCullMode() const;
	
	const vk::FrontFace& getFrontFace() const;
	
	VKPipelineVertexInputLayoutInfo& getVertexInputLayoutInfo();
	const VKPipelineVertexInputLayoutInfo& getVertexInputLayoutInfo() const;
	
	VKPipelineAttachmentInfo& getPipelineAttachmentInfo();
	const VKPipelineAttachmentInfo& getPipelineAttachmentInfo() const;
	
	bool hasGeometryShader() const;
	const std::filesystem::path& getGeometryShader() const;
	void setGeometryShader(const std::filesystem::path& path);
	
	bool hasFragmentShader() const;
	const std::filesystem::path& getFragmentShader() const;
	void setFragmentShader(const std::filesystem::path& path);
	
	bool hasStaticViewport() const;
	const VKPipelineViewport& getStaticViewport() const;
	void setStaticViewport(const VKPipelineViewport& staticViewport);
	
	bool hasStaticScissor() const;
	const VKPipelineScissor& getStaticScissor() const;
	void setStaticScissor(const VKPipelineScissor& staticScissor);
	
	void setRasterizationSampleCount(vk::SampleCountFlagBits sampleCount);
	const vk::SampleCountFlagBits& getRasterizationSampleCount() const;
	
	void setViewMask(uint32_t viewMask);
	const uint32_t& getViewMask() const;
	
private:
	VKPtr<VKPipelineLayout> _pipelineLayout;
	std::filesystem::path _vertexShader;
	vk::PrimitiveTopology _primitiveTopology;
	vk::CullModeFlags _cullMode;
	vk::FrontFace _frontFace;
	VKPipelineVertexInputLayoutInfo _vertexInputLayoutInfo;
	VKPipelineAttachmentInfo _pipelineAttachmentInfo;
	std::optional<std::filesystem::path> _geometryShader;
	std::optional<std::filesystem::path> _fragmentShader;
	std::optional<VKPipelineViewport> _staticViewport;
	std::optional<VKPipelineScissor> _staticScissor;
	vk::SampleCountFlagBits _rasterizationSampleCount = vk::SampleCountFlagBits::e1;
	uint32_t _viewMask = 0;
};