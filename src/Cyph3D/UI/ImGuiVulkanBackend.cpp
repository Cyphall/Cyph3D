#include "ImGuiVulkanBackend.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/VKObject/VKSwapchain.h"
#include "Cyph3D/Window.h"

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

void ImGuiVulkanBackend::renderDrawData(ImDrawData* drawData, const VKPtr<VKCommandBuffer>& commandBuffer, const VKPtr<VKImage>& outputImage)
{
	glm::uvec2 framebufferSize = outputImage->getSize(0);

	_vertexBuffer->resizeSmart(drawData->TotalVtxCount);
	_indexBuffer->resizeSmart(drawData->TotalIdxCount);

	ImDrawVert* vertexBufferPtr = _vertexBuffer->getHostPointer();
	ImDrawIdx* indexBufferPtr = _indexBuffer->getHostPointer();
	for (int i = 0; i < drawData->CmdListsCount; i++)
	{
		const ImDrawList* cmdList = drawData->CmdLists[i];

		std::copy(cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Data + cmdList->VtxBuffer.Size, vertexBufferPtr);
		std::copy(cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Data + cmdList->IdxBuffer.Size, indexBufferPtr);

		vertexBufferPtr += cmdList->VtxBuffer.Size;
		indexBufferPtr += cmdList->IdxBuffer.Size;
	}

	VKRenderingInfo renderingInfo(framebufferSize);

	renderingInfo.addColorAttachment(outputImage)
		.setLoadOpClear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
		.setStoreOpStore();

	commandBuffer->beginRendering(renderingInfo);

	setupRenderState(drawData, commandBuffer, _vertexBuffer.getCurrent(), _indexBuffer.getCurrent(), framebufferSize);

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
					setupRenderState(drawData, commandBuffer, _vertexBuffer.getCurrent(), _indexBuffer.getCurrent(), framebufferSize);
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

				const VKPtr<VKImage>& texture = *static_cast<const VKPtr<VKImage>*>(pcmd->TextureId);

				if (texture->getLayout(0, 0) != vk::ImageLayout::eReadOnlyOptimal)
				{
					Logger::error("VKImage passed to ImGui has the wrong layout");
				}

				if (texture.get() == _fontsTexture.get())
				{
					commandBuffer->pushDescriptor(0, 0, _fontsTexture, _fontsSampler);
				}
				else
				{
					commandBuffer->pushDescriptor(0, 0, texture, vk::ImageViewType::e2D, {0, 0}, {0, 0}, texture->getInfo().getFormat(), _textureSampler);
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
	info.addBinding(vk::DescriptorType::eCombinedImageSampler, 1);
	_imageSamplerDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void ImGuiVulkanBackend::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(_imageSamplerDescriptorSetLayout);
	info.setPushConstantLayout<PushConstantData>();

	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void ImGuiVulkanBackend::createPipeline()
{
	VKGraphicsPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/imgui/imgui.vert",
		vk::PrimitiveTopology::eTriangleList,
		vk::CullModeFlagBits::eNone,
		vk::FrontFace::eCounterClockwise
	);

	info.setFragmentShader("resources/shaders/internal/imgui/imgui.frag");

	info.getVertexInputLayoutInfo().defineSlot(0, sizeof(ImDrawVert), vk::VertexInputRate::eVertex);
	info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos));
	info.getVertexInputLayoutInfo().defineAttribute(0, 1, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, uv));
	info.getVertexInputLayoutInfo().defineAttribute(0, 2, vk::Format::eR8G8B8A8Unorm, offsetof(ImDrawVert, col));

	VKPipelineBlendingInfo blendingInfo{
		.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
		.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		.colorBlendOp = vk::BlendOp::eAdd,
		.srcAlphaBlendFactor = vk::BlendFactor::eOne,
		.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
		.alphaBlendOp = vk::BlendOp::eAdd
	};

	info.getPipelineAttachmentInfo().addColorAttachment(Engine::getWindow().getSwapchain().getFormat(), blendingInfo);

	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void ImGuiVulkanBackend::createFontsTexture()
{
	ImGuiIO& io = ImGui::GetIO();

	uint8_t* data;
	int width, height;
	io.Fonts->GetTexDataAsAlpha8(&data, &width, &height);

	uint64_t dataSize = width * height;

	VKBufferInfo bufferInfo(dataSize, vk::BufferUsageFlagBits::eTransferSrc);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);

	VKPtr<VKBuffer<uint8_t>> stagingBuffer = VKBuffer<uint8_t>::create(Engine::getVKContext(), bufferInfo);

	std::copy(data, data + dataSize, stagingBuffer->getHostPointer());

	VKImageInfo imageInfo(
		vk::Format::eR8Unorm,
		glm::uvec2(width, height),
		1,
		1,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.setName("ImGui fonts texture");
	imageInfo.setSwizzle({vk::ComponentSwizzle::eOne, vk::ComponentSwizzle::eOne, vk::ComponentSwizzle::eOne, vk::ComponentSwizzle::eR});

	_fontsTexture = VKImage::create(Engine::getVKContext(), imageInfo);

	Engine::getVKContext().executeImmediate(
		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
		{
			commandBuffer->imageMemoryBarrier(
				_fontsTexture,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::ImageLayout::eTransferDstOptimal
			);

			commandBuffer->copyBufferToImage(stagingBuffer, 0, _fontsTexture, 0, 0);

			commandBuffer->imageMemoryBarrier(
				_fontsTexture,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eFragmentShader,
				vk::AccessFlagBits2::eShaderSampledRead,
				vk::ImageLayout::eReadOnlyOptimal
			);
		}
	);

	io.Fonts->SetTexID(static_cast<ImTextureID>(&_fontsTexture));
}

void ImGuiVulkanBackend::createBuffers()
{
	{
		VKResizableBufferInfo info(vk::BufferUsageFlagBits::eVertexBuffer);
		info.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		info.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
		info.addPreferredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		info.setName("ImGui vertex buffer");

		_vertexBuffer = VKDynamic<VKResizableBuffer<ImDrawVert>>(
			Engine::getVKContext(),
			[&](VKContext& context, int index)
			{
				return VKResizableBuffer<ImDrawVert>::create(context, info);
			}
		);
	}

	{
		VKResizableBufferInfo info(vk::BufferUsageFlagBits::eIndexBuffer);
		info.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		info.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
		info.addPreferredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		info.setName("ImGui index buffer");

		_indexBuffer = VKDynamic<VKResizableBuffer<ImDrawIdx>>(
			Engine::getVKContext(),
			[&](VKContext& context, int index)
			{
				return VKResizableBuffer<ImDrawIdx>::create(context, info);
			}
		);
	}
}

void ImGuiVulkanBackend::setupRenderState(
	ImDrawData* drawData,
	const VKPtr<VKCommandBuffer>& commandBuffer,
	const VKPtr<VKResizableBuffer<ImDrawVert>>& vertexBuffer,
	const VKPtr<VKResizableBuffer<ImDrawIdx>>& indexBuffer,
	glm::uvec2 viewportSize
)
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

	PushConstantData pushConstantData{};
	pushConstantData.scale.x = 2.0f / drawData->DisplaySize.x;
	pushConstantData.scale.y = 2.0f / drawData->DisplaySize.y;
	pushConstantData.offset.x = -1.0f - drawData->DisplayPos.x * pushConstantData.scale.x;
	pushConstantData.offset.y = -1.0f - drawData->DisplayPos.y * pushConstantData.scale.y;
	commandBuffer->pushConstants(pushConstantData);
}