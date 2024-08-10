#pragma once

#include "Cyph3D/VKObject/VKDynamic.h"

#include <glm/glm.hpp>
#include <imgui.h>

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

	void renderDrawData(const ImDrawData* drawData, const std::shared_ptr<VKCommandBuffer>& commandBuffer, const std::shared_ptr<VKImage>& outputImage);

private:
	struct PushConstantData
	{
		alignas(4) glm::vec2 scale;
		alignas(4) glm::vec2 offset;
	};

	std::shared_ptr<VKSampler> _textureSampler;
	std::shared_ptr<VKSampler> _fontsSampler;

	std::shared_ptr<VKDescriptorSetLayout> _imageSamplerDescriptorSetLayout;

	std::shared_ptr<VKPipelineLayout> _pipelineLayout;

	std::shared_ptr<VKGraphicsPipeline> _pipeline;

	std::shared_ptr<VKImage> _fontsTexture;

	VKDynamic<VKResizableBuffer<ImDrawVert>> _vertexBuffer;
	VKDynamic<VKResizableBuffer<ImDrawIdx>> _indexBuffer;

	void createSamplers();
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createFontsTexture();
	void createBuffers();

	void setupRenderState(
		const ImDrawData* drawData,
		const std::shared_ptr<VKCommandBuffer>& commandBuffer,
		const std::shared_ptr<VKResizableBuffer<ImDrawVert>>& vertexBuffer,
		const std::shared_ptr<VKResizableBuffer<ImDrawIdx>>& indexBuffer,
		glm::uvec2 viewportSize
	);
};