#pragma once

#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>
#include <imgui.h>
#include <memory>

class VKCommandBuffer;
class VKDescriptorSetLayout;
class VKSampler;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;
template<typename T>
class VKResizableBuffer;

class ImGuiVulkanBackend
{
public:
	ImGuiVulkanBackend();
	~ImGuiVulkanBackend();
	
	void renderDrawData(ImDrawData* drawData, const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImage>& outputImage);

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