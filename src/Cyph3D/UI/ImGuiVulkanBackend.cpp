#include "ImGuiVulkanBackend.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Window.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/VKObject/VKSwapchain.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"

ImGuiVulkanBackend::ImGuiVulkanBackend()
{
	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = "ImGuiVulkanBackend";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	
	createSamplers();
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createFontsTexture();
	createBuffers();
}

ImGuiVulkanBackend::~ImGuiVulkanBackend()
{
	ImGuiIO& io = ImGui::GetIO();
	
	io.Fonts->SetTexID(static_cast<ImTextureID>(nullptr));
	
	io.BackendRendererName = nullptr;
	io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
}

void ImGuiVulkanBackend::renderDrawData(ImDrawData* drawData, const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImageView>& outputImageView)
{
	glm::uvec2 framebufferSize = outputImageView->getImage()->getSize(0);
	
	_vertexBuffer->resizeSmart(drawData->TotalVtxCount);
	_indexBuffer->resizeSmart(drawData->TotalIdxCount);
	
	ImDrawVert* vertexBufferPtr = _vertexBuffer->map();
	ImDrawIdx* indexBufferPtr = _indexBuffer->map();
	
	for (int i = 0; i < drawData->CmdListsCount; i++)
	{
		const ImDrawList* cmdList = drawData->CmdLists[i];
		
		std::copy(cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Data + cmdList->VtxBuffer.Size, vertexBufferPtr);
		std::copy(cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Data + cmdList->IdxBuffer.Size, indexBufferPtr);
		
		vertexBufferPtr += cmdList->VtxBuffer.Size;
		indexBufferPtr += cmdList->IdxBuffer.Size;
	}
	
	_vertexBuffer->unmap();
	_indexBuffer->unmap();
	
	vk::RenderingAttachmentInfo colorAttachment;
	colorAttachment.imageView = outputImageView->getHandle();
	colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	colorAttachment.resolveImageView = nullptr;
	colorAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.clearValue.color.float32[0] = 0.0f;
	colorAttachment.clearValue.color.float32[1] = 0.0f;
	colorAttachment.clearValue.color.float32[2] = 0.0f;
	colorAttachment.clearValue.color.float32[3] = 1.0f;
	
	vk::RenderingInfo renderingInfo;
	renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
	renderingInfo.renderArea.extent = vk::Extent2D(framebufferSize.x, framebufferSize.y);
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;
	
	commandBuffer->beginRendering(renderingInfo);
	
	setupRenderState(drawData, commandBuffer, _vertexBuffer.getVKPtr(), _indexBuffer.getVKPtr(), framebufferSize);
	
	glm::vec2 clipOffset = drawData->DisplayPos;
	glm::vec2 clipScale = drawData->FramebufferScale;
	
	int globalVertexOffset = 0;
	int globalIndexOffset = 0;
	for (int i = 0; i < drawData->CmdListsCount; i++)
	{
		const ImDrawList* cmdList = drawData->CmdLists[i];
		for (int j = 0; j < cmdList->CmdBuffer.Size; j++)
		{
			const ImDrawCmd* pcmd = &cmdList->CmdBuffer[j];
			if (pcmd->UserCallback != nullptr)
			{
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
				{
					setupRenderState(drawData, commandBuffer, _vertexBuffer.getVKPtr(), _indexBuffer.getVKPtr(), framebufferSize);
				}
				else
				{
					pcmd->UserCallback(cmdList, pcmd);
				}
			}
			else
			{
				glm::vec2 clipMin = {(pcmd->ClipRect.x - clipOffset.x) * clipScale.x, (pcmd->ClipRect.y - clipOffset.y) * clipScale.y};
				glm::vec2 clipMax = {(pcmd->ClipRect.z - clipOffset.x) * clipScale.x, (pcmd->ClipRect.w - clipOffset.y) * clipScale.y};
				
				clipMin = glm::max(clipMin, {0.0f, 0.0f});
				clipMax = glm::min(clipMax, glm::vec2(framebufferSize));
				if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
				{
					continue;
				}
				
				VKPipelineScissor scissor;
				scissor.offset.x = clipMin.x;
				scissor.offset.y = clipMin.y;
				scissor.size.x = clipMax.x - clipMin.x;
				scissor.size.y = clipMax.y - clipMin.y;
				commandBuffer->setScissor(scissor);
				
				const VKPtr<VKImageView>& textureView = *static_cast<const VKPtr<VKImageView>*>(pcmd->TextureId);
				
				if (textureView->getImage()->getLayout(0, 0) != vk::ImageLayout::eReadOnlyOptimal)
				{
					Logger::error("VKImage passed to ImGui has the wrong layout");
				}
				
				if (textureView.get() == _fontsTextureView.get())
				{
					commandBuffer->pushDescriptor(0, 0, _fontsTextureView);
					commandBuffer->pushDescriptor(0, 1, _fontsSampler);
				}
				else
				{
					commandBuffer->pushDescriptor(0, 0, textureView);
					commandBuffer->pushDescriptor(0, 1, _textureSampler);
				}
				
				commandBuffer->drawIndexed(pcmd->ElemCount, pcmd->IdxOffset + globalIndexOffset, pcmd->VtxOffset + globalVertexOffset);
			}
		}
		globalIndexOffset += cmdList->IdxBuffer.Size;
		globalVertexOffset += cmdList->VtxBuffer.Size;
	}
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
}

void ImGuiVulkanBackend::createSamplers()
{
	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.minFilter = vk::Filter::eLinear;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		createInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = false;
		createInfo.maxAnisotropy = 1;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		createInfo.unnormalizedCoordinates = false;
		
		_textureSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}
	
	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.minFilter = vk::Filter::eLinear;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		createInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = false;
		createInfo.maxAnisotropy = 1;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		createInfo.unnormalizedCoordinates = false;
		
		_fontsSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}
}

void ImGuiVulkanBackend::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.registerBinding(0, vk::DescriptorType::eSampledImage, 1);
	info.registerBinding(1, vk::DescriptorType::eSampler, 1);
	_imageSamplerDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void ImGuiVulkanBackend::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.registerDescriptorSetLayout(_imageSamplerDescriptorSetLayout);
	info.registerPushConstantLayout<PushConstantData>(vk::ShaderStageFlagBits::eVertex);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void ImGuiVulkanBackend::createPipeline()
{
	VKGraphicsPipelineInfo info;
	info.vertexShaderFile = "resources/shaders/internal/imgui/imgui.vert";
	info.geometryShaderFile = std::nullopt;
	info.fragmentShaderFile = "resources/shaders/internal/imgui/imgui.frag";
	
	info.vertexInputLayoutInfo.defineAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos));
	info.vertexInputLayoutInfo.defineAttribute(0, 1, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv));
	info.vertexInputLayoutInfo.defineAttribute(0, 2, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col));
	info.vertexInputLayoutInfo.defineSlot(0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex);
	
	info.vertexTopology = vk::PrimitiveTopology::eTriangleList;
	
	info.pipelineLayout = _pipelineLayout;
	
	info.viewport = std::nullopt;
	
	info.rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
	info.rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
	
	VKPipelineBlendingInfo blendingInfo;
	blendingInfo.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
	blendingInfo.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	blendingInfo.colorBlendOp = vk::BlendOp::eAdd;
	blendingInfo.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	blendingInfo.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
	blendingInfo.alphaBlendOp = vk::BlendOp::eAdd;
	
	info.pipelineAttachmentInfo.registerColorAttachment(0, Engine::getWindow().getSwapchain().getFormat(), blendingInfo);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void ImGuiVulkanBackend::createFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();
	
	uint8_t* data;
	int width, height;
	io.Fonts->GetTexDataAsAlpha8(&data, &width, &height);
	
	uint64_t dataSize = width * height;
	
	VKPtr<VKBuffer<uint8_t>> stagingBuffer = VKBuffer<uint8_t>::create(
		Engine::getVKContext(),
		dataSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryPropertyFlagBits::eHostCached);
	
	uint8_t* ptr = stagingBuffer->map();
	std::copy(data, data + dataSize, ptr);
	stagingBuffer->unmap();
	
	_fontsTexture = VKImage::create(
		Engine::getVKContext(),
		vk::Format::eR8Unorm,
		glm::uvec2(width, height),
		1,
		1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
		vk::ImageAspectFlagBits::eColor,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	Engine::getVKContext().executeImmediate(
		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
		{
			commandBuffer->imageMemoryBarrier(
				_fontsTexture,
				vk::PipelineStageFlagBits2::eHost,
				vk::AccessFlagBits2::eHostWrite,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eTransferDstOptimal,
				0,
				0);
			
			commandBuffer->copyBufferToImage(stagingBuffer, 0, _fontsTexture, 0, 0);
			
			commandBuffer->imageMemoryBarrier(
				_fontsTexture,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal,
				0,
				0);
		}
	);
	
	_fontsTextureView = VKImageView::create(
		Engine::getVKContext(),
		_fontsTexture,
		vk::ImageViewType::e2D,
		std::array<vk::ComponentSwizzle, 4>{vk::ComponentSwizzle::eOne, vk::ComponentSwizzle::eOne, vk::ComponentSwizzle::eOne, vk::ComponentSwizzle::eR});
	
	io.Fonts->SetTexID(static_cast<ImTextureID>(&_fontsTextureView));
}

void ImGuiVulkanBackend::createBuffers()
{
	_vertexBuffer = VKResizableBuffer<ImDrawVert>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_indexBuffer = VKResizableBuffer<ImDrawIdx>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eIndexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
}

void ImGuiVulkanBackend::setupRenderState(
	ImDrawData* drawData,
	const VKPtr<VKCommandBuffer>& commandBuffer,
	const VKPtr<VKResizableBuffer<ImDrawVert>>& vertexBuffer,
	const VKPtr<VKResizableBuffer<ImDrawIdx>>& indexBuffer,
	glm::uvec2 viewportSize)
{
	commandBuffer->bindPipeline(_pipeline);
	
	if (drawData->TotalVtxCount > 0)
	{
		commandBuffer->bindVertexBuffer(0, vertexBuffer->getBuffer());
		commandBuffer->bindIndexBuffer(indexBuffer->getBuffer());
	}
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = viewportSize;
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	PushConstantData pushConstantData;
	pushConstantData.scale.x = 2.0f / drawData->DisplaySize.x;
	pushConstantData.scale.y = 2.0f / drawData->DisplaySize.y;
	pushConstantData.offset.x = -1.0f - drawData->DisplayPos.x * pushConstantData.scale.x;
	pushConstantData.offset.y = -1.0f - drawData->DisplayPos.y * pushConstantData.scale.y;
	commandBuffer->pushConstants(vk::ShaderStageFlagBits::eVertex, pushConstantData);
}