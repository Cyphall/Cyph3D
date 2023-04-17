#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"

#include <glm/glm.hpp>
#include <memory>
#include <imgui.h>

class VKCommandBuffer;
class VKDescriptorSetLayout;
class VKSampler;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;
class VKImageView;
template<typename T>
class VKResizableBuffer;

class ImGuiVulkanBackend
{
public:
	ImGuiVulkanBackend();
	~ImGuiVulkanBackend();
	
	void renderDrawData(ImDrawData* drawData, const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImageView>& outputImageView);

private:
	struct PushConstantData
	{
		alignas(4) glm::vec2 scale;
		alignas(4) glm::vec2 offset;
	};
	
	VKPtr<VKSampler> _textureSampler;
	VKPtr<VKSampler> _fontsSampler;
	
	VKPtr<VKDescriptorSetLayout> _imageSamplerDescriptorSetLayout;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	
	VKPtr<VKGraphicsPipeline> _pipeline;
	
	VKPtr<VKImage> _fontsTexture;
	VKPtr<VKImageView> _fontsTextureView;
	
	VKDynamic<VKResizableBuffer<ImDrawVert>> _vertexBuffer;
	VKDynamic<VKResizableBuffer<ImDrawIdx>> _indexBuffer;
	
	void createSamplers();
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createFontsTexture();
	void createBuffers();
	
	void setupRenderState(
		ImDrawData* drawData,
		const VKPtr<VKCommandBuffer>& commandBuffer,
		const VKPtr<VKResizableBuffer<ImDrawVert>>& vertexBuffer,
		const VKPtr<VKResizableBuffer<ImDrawIdx>>& indexBuffer,
		glm::uvec2 viewportSize);
};