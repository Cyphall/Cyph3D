#include "ImGuiVulkanBackend.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/VKObject/Buffer/VKResizableBuffer.h>
#include <Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h>
#include <Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h>
#include <Cyph3D/VKObject/Image/VKImage.h>
#include <Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h>
#include <Cyph3D/VKObject/Pipeline/VKPipelineLayout.h>
#include <Cyph3D/VKObject/Sampler/VKSampler.h>
#include <Cyph3D/VKObject/VKContext.h>
#include <Cyph3D/VKObject/VKDynamic.h>
#include <Cyph3D/VKObject/VKSwapchain.h>
#include <Cyph3D/Window.h>

#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

namespace
{
struct PushConstantData
{
	glm::vec2 scale;
	glm::vec2 offset;
};

struct ImGui_ImplVKObject_BackendData
{
	std::shared_ptr<c3d::VKSampler> samplerLinear;
	std::shared_ptr<c3d::VKSampler> samplerNearest;
	std::shared_ptr<c3d::VKDescriptorSetLayout> descriptorSetLayout;
	std::shared_ptr<c3d::VKPipelineLayout> pipelineLayout;
	std::shared_ptr<c3d::VKGraphicsPipeline> pipeline;
	c3d::VKDynamic<c3d::VKResizableBuffer<ImDrawVert>> vertexBuffer;
	c3d::VKDynamic<c3d::VKResizableBuffer<ImDrawIdx>> indexBuffer;

	// ImTextureID -> VKImage map
	std::vector<std::shared_ptr<c3d::VKImage>> referencedImages;
};

struct ImGui_ImplVKObject_BackendTextureData
{
	std::shared_ptr<c3d::VKImage> image;
};

void ImGui_ImplVKObject_CreateTexture(ImTextureData& texture)
{
	vk::Format format{};
	std::array<vk::ComponentSwizzle, 4> swizzle{};
	switch (texture.Format)
	{
	case ImTextureFormat_RGBA32:
		format = vk::Format::eR8G8B8A8Unorm;
		swizzle = {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA};
		break;
	case ImTextureFormat_Alpha8:
		format = vk::Format::eR8Unorm;
		swizzle = {vk::ComponentSwizzle::eOne, vk::ComponentSwizzle::eOne, vk::ComponentSwizzle::eOne, vk::ComponentSwizzle::eR};
		break;
	default:
		throw std::runtime_error(std::format("Unknown texture format: {}", magic_enum::enum_name(texture.Format)));
	}

	texture.BackendUserData = new ImGui_ImplVKObject_BackendTextureData{};

	ImGui_ImplVKObject_BackendTextureData& btd = *static_cast<ImGui_ImplVKObject_BackendTextureData*>(texture.BackendUserData);

	c3d::VKImageInfo imageInfo(
		format,
		{texture.Width, texture.Height},
		1,
		1,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
	);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	imageInfo.setName("ImGui texture");
	imageInfo.setSwizzle(swizzle);

	btd.image = c3d::VKImage::create(c3d::Engine::getVKContext(), imageInfo);

	texture.SetTexID(c3d::ImGui_ImplVKObject_ToTextureID(btd.image));
}

//TODO: add support for upload subregion
void ImGui_ImplVKObject_UploadTexture(const std::shared_ptr<c3d::VKCommandBuffer>& commandBuffer, ImTextureData& texture)
{
	ImGui_ImplVKObject_BackendTextureData& btd = *static_cast<ImGui_ImplVKObject_BackendTextureData*>(texture.BackendUserData);

	c3d::VKBufferInfo bufferInfo(texture.GetSizeInBytes(), vk::BufferUsageFlagBits::eTransferSrc);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);

	std::shared_ptr<c3d::VKBuffer<std::byte>> stagingBuffer = c3d::VKBuffer<std::byte>::create(c3d::Engine::getVKContext(), bufferInfo);

	std::memcpy(stagingBuffer->getHostPointer(), texture.GetPixels(), texture.GetSizeInBytes());

	commandBuffer->bufferMemoryBarrier(
		stagingBuffer,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferRead
	);

	commandBuffer->imageMemoryBarrier(
		btd.image,
		vk::PipelineStageFlagBits2::eCopy,
		vk::AccessFlagBits2::eTransferWrite,
		vk::ImageLayout::eTransferDstOptimal
	);

	commandBuffer->copyBufferToImage(stagingBuffer, 0, btd.image, 0, 0);

	commandBuffer->imageMemoryBarrier(
		btd.image,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::ImageLayout::eReadOnlyOptimal
	);
}

void ImGui_ImplVKObject_DestroyTexture(ImTextureData& texture)
{
	delete static_cast<ImGui_ImplVKObject_BackendTextureData*>(texture.BackendUserData);
	texture.BackendUserData = nullptr;

	texture.SetTexID(ImTextureID_Invalid);
}

void ImGui_ImplVKObject_UpdateTexture(const std::shared_ptr<c3d::VKCommandBuffer>& commandBuffer, ImTextureData& texture)
{
	switch (texture.Status)
	{
	case ImTextureStatus_WantCreate:
		ImGui_ImplVKObject_CreateTexture(texture);
		ImGui_ImplVKObject_UploadTexture(commandBuffer, texture);
		texture.SetStatus(ImTextureStatus_OK);
		break;
	case ImTextureStatus_WantUpdates:
		ImGui_ImplVKObject_UploadTexture(commandBuffer, texture);
		texture.SetStatus(ImTextureStatus_OK);
		break;
	case ImTextureStatus_WantDestroy:
		ImGui_ImplVKObject_DestroyTexture(texture);
		texture.SetStatus(ImTextureStatus_Destroyed);
		break;
	default:
		throw std::logic_error(std::format("Unhandled texture state: {}", magic_enum::enum_name(texture.Status)));
	}
}

void ImGui_ImplVKObject_SetupRenderState(const ImDrawData& drawData, const std::shared_ptr<c3d::VKCommandBuffer>& commandBuffer, glm::vec2 viewportSize)
{
	ImGui_ImplVKObject_BackendData& bd = *static_cast<ImGui_ImplVKObject_BackendData*>(ImGui::GetIO().BackendRendererUserData);

	commandBuffer->bindPipeline(bd.pipeline);

	if (drawData.TotalVtxCount > 0)
	{
		commandBuffer->bindVertexBuffer(0, bd.vertexBuffer->getBuffer());
		commandBuffer->bindIndexBuffer(bd.indexBuffer->getBuffer());
	}

	c3d::VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = viewportSize;
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);

	PushConstantData pushConstantData{};
	pushConstantData.scale.x = 2.0f / drawData.DisplaySize.x;
	pushConstantData.scale.y = 2.0f / drawData.DisplaySize.y;
	pushConstantData.offset.x = -1.0f - drawData.DisplayPos.x * pushConstantData.scale.x;
	pushConstantData.offset.y = -1.0f - drawData.DisplayPos.y * pushConstantData.scale.y;
	commandBuffer->pushConstants(pushConstantData);

	commandBuffer->pushDescriptor(0, 0, bd.samplerLinear);
}

void ImGui_ImplVKObject_DrawCallback_ResetRenderState(const ImDrawList*, const ImDrawCmd*)
{}

void ImGui_ImplVKObject_DrawCallback_SetSamplerLinear(const ImDrawList*, const ImDrawCmd*)
{}

void ImGui_ImplVKObject_DrawCallback_SetSamplerNearest(const ImDrawList*, const ImDrawCmd*)
{}
}

void c3d::ImGui_ImplVKObject_Init()
{
	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = "ImGuiVulkanBackend";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
	io.BackendRendererUserData = new ImGui_ImplVKObject_BackendData{};

	ImGui_ImplVKObject_BackendData& bd = *static_cast<ImGui_ImplVKObject_BackendData*>(io.BackendRendererUserData);

	uint32_t maxImageDimension = Engine::getVKContext().getVulkan10Properties().limits.maxImageDimension2D;

	ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
	platformIO.Renderer_TextureMaxWidth = static_cast<int>(maxImageDimension);
	platformIO.Renderer_TextureMaxHeight = static_cast<int>(maxImageDimension);
	platformIO.DrawCallback_ResetRenderState = ImGui_ImplVKObject_DrawCallback_ResetRenderState;
	platformIO.DrawCallback_SetSamplerLinear = ImGui_ImplVKObject_DrawCallback_SetSamplerLinear;
	platformIO.DrawCallback_SetSamplerNearest = ImGui_ImplVKObject_DrawCallback_SetSamplerNearest;

	// Linear sampler
	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.minFilter = vk::Filter::eLinear;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		createInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = false;
		createInfo.maxAnisotropy = 1;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		createInfo.unnormalizedCoordinates = false;

		bd.samplerLinear = VKSampler::create(Engine::getVKContext(), createInfo);
	}

	// Nearest sampler
	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eNearest;
		createInfo.minFilter = vk::Filter::eNearest;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
		createInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = false;
		createInfo.maxAnisotropy = 1;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		createInfo.unnormalizedCoordinates = false;

		bd.samplerNearest = VKSampler::create(Engine::getVKContext(), createInfo);
	}

	// Descriptor set layout
	{
		VKDescriptorSetLayoutInfo info(true);
		info.addBinding(vk::DescriptorType::eSampler, 1);
		info.addBinding(vk::DescriptorType::eSampledImage, 1);
		bd.descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
	}

	// Pipeline layout
	{
		VKPipelineLayoutInfo info;
		info.addDescriptorSetLayout(bd.descriptorSetLayout);
		info.setPushConstantLayout<PushConstantData>();

		bd.pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}

	// Pipeline
	{
		VKGraphicsPipelineInfo info(
			bd.pipelineLayout,
			"imgui/imgui.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eNone,
			vk::FrontFace::eCounterClockwise
		);

		info.setFragmentShader("imgui/imgui.frag");

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

		bd.pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}

	// Vertex buffer
	{
		VKResizableBufferInfo info(vk::BufferUsageFlagBits::eVertexBuffer);
		info.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		info.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
		info.addPreferredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		info.setName("ImGui vertex buffer");

		bd.vertexBuffer = VKDynamic<VKResizableBuffer<ImDrawVert>>(
			Engine::getVKContext(),
			[&](VKContext& context, int index)
			{
				return VKResizableBuffer<ImDrawVert>::create(context, info);
			}
		);
	}

	// Index buffer
	{
		VKResizableBufferInfo info(vk::BufferUsageFlagBits::eIndexBuffer);
		info.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		info.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
		info.addPreferredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		info.setName("ImGui index buffer");

		bd.indexBuffer = VKDynamic<VKResizableBuffer<ImDrawIdx>>(
			Engine::getVKContext(),
			[&](VKContext& context, int index)
			{
				return VKResizableBuffer<ImDrawIdx>::create(context, info);
			}
		);
	}
}

void c3d::ImGui_ImplVKObject_NewFrame()
{
	ImGui_ImplVKObject_BackendData& bd = *static_cast<ImGui_ImplVKObject_BackendData*>(ImGui::GetIO().BackendRendererUserData);

	bd.referencedImages.clear();

	for (ImTextureData* texture : ImGui::GetPlatformIO().Textures)
	{
		if (!texture->BackendUserData)
			continue;

		ImGui_ImplVKObject_BackendTextureData& btd = *static_cast<ImGui_ImplVKObject_BackendTextureData*>(texture->BackendUserData);
		texture->SetTexID(ImGui_ImplVKObject_ToTextureID(btd.image));
	}
}

void c3d::ImGui_ImplVKObject_RenderDrawData(const ImDrawData& drawData, const std::shared_ptr<VKCommandBuffer>& commandBuffer, const std::shared_ptr<VKImage>& outputImage)
{
	ImGui_ImplVKObject_BackendData& bd = *static_cast<ImGui_ImplVKObject_BackendData*>(ImGui::GetIO().BackendRendererUserData);

	if (drawData.Textures != nullptr)
		for (ImTextureData* texture : *drawData.Textures)
			if (texture->Status != ImTextureStatus_OK)
				ImGui_ImplVKObject_UpdateTexture(commandBuffer, *texture);

	glm::uvec2 framebufferSize = outputImage->getSize(0);

	bd.vertexBuffer->resizeSmart(drawData.TotalVtxCount);
	bd.indexBuffer->resizeSmart(drawData.TotalIdxCount);

	ImDrawVert* vertexBufferPtr = bd.vertexBuffer->getHostPointer();
	ImDrawIdx* indexBufferPtr = bd.indexBuffer->getHostPointer();
	for (int i = 0; i < drawData.CmdListsCount; i++)
	{
		const ImDrawList& cmdList = *drawData.CmdLists[i];

		std::copy_n(cmdList.VtxBuffer.Data, cmdList.VtxBuffer.Size, vertexBufferPtr);
		std::copy_n(cmdList.IdxBuffer.Data, cmdList.IdxBuffer.Size, indexBufferPtr);

		vertexBufferPtr += cmdList.VtxBuffer.Size;
		indexBufferPtr += cmdList.IdxBuffer.Size;
	}

	VKRenderingInfo renderingInfo(framebufferSize);

	renderingInfo.addColorAttachment(outputImage)
		.setLoadOpClear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f))
		.setStoreOpStore();

	commandBuffer->beginRendering(renderingInfo);

	ImGui_ImplVKObject_SetupRenderState(drawData, commandBuffer, framebufferSize);

	glm::vec2 clipOffset = drawData.DisplayPos;
	glm::vec2 clipScale = drawData.FramebufferScale;

	int globalVertexOffset = 0;
	int globalIndexOffset = 0;
	for (const ImDrawList* cmdList : drawData.CmdLists)
	{
		for (const ImDrawCmd& cmd : cmdList->CmdBuffer)
		{
			if (cmd.UserCallback != nullptr)
			{
				if (cmd.UserCallback == ImGui_ImplVKObject_DrawCallback_ResetRenderState)
					ImGui_ImplVKObject_SetupRenderState(drawData, commandBuffer, framebufferSize);
				else if (cmd.UserCallback == ImGui_ImplVKObject_DrawCallback_SetSamplerLinear)
					commandBuffer->pushDescriptor(0, 0, bd.samplerLinear);
				else if (cmd.UserCallback == ImGui_ImplVKObject_DrawCallback_SetSamplerNearest)
					commandBuffer->pushDescriptor(0, 0, bd.samplerNearest);
				else
					cmd.UserCallback(cmdList, &cmd);
			}
			else
			{
				glm::vec2 clipMin = {(cmd.ClipRect.x - clipOffset.x) * clipScale.x, (cmd.ClipRect.y - clipOffset.y) * clipScale.y};
				glm::vec2 clipMax = {(cmd.ClipRect.z - clipOffset.x) * clipScale.x, (cmd.ClipRect.w - clipOffset.y) * clipScale.y};

				clipMin = glm::max(clipMin, {0.0f, 0.0f});
				clipMax = glm::min(clipMax, glm::vec2{framebufferSize});
				if (glm::any(glm::lessThanEqual(clipMax, clipMin)))
				{
					continue;
				}

				VKPipelineScissor scissor;
				scissor.offset.x = static_cast<int32_t>(clipMin.x);
				scissor.offset.y = static_cast<int32_t>(clipMin.y);
				scissor.size.x = static_cast<uint32_t>(clipMax.x - clipMin.x);
				scissor.size.y = static_cast<uint32_t>(clipMax.y - clipMin.y);
				commandBuffer->setScissor(scissor);

				const std::shared_ptr<VKImage>& image = bd.referencedImages[cmd.GetTexID() - 1];
				if (image->getState(0, 0).layout != vk::ImageLayout::eReadOnlyOptimal)
					spdlog::error("VKImage passed to ImGui has the wrong layout.");

				commandBuffer->pushDescriptor(0, 1, image, vk::ImageViewType::e2D, {0, 0}, {0, 0}, image->getInfo().getFormat());

				commandBuffer->drawIndexed(cmd.ElemCount, cmd.IdxOffset + globalIndexOffset, cmd.VtxOffset + globalVertexOffset);
			}
		}
		globalIndexOffset += cmdList->IdxBuffer.Size;
		globalVertexOffset += cmdList->VtxBuffer.Size;
	}

	commandBuffer->unbindPipeline();

	commandBuffer->endRendering();
}

void c3d::ImGui_ImplVKObject_Shutdown()
{
	for (ImTextureData* texture : ImGui::GetPlatformIO().Textures)
	{
		if (texture->RefCount == 1)
		{
			ImGui_ImplVKObject_DestroyTexture(*texture);
			texture->SetStatus(ImTextureStatus_Destroyed);
		}
	}

	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = nullptr;
	io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
	io.BackendFlags &= ~ImGuiBackendFlags_RendererHasTextures;
	delete static_cast<ImGui_ImplVKObject_BackendData*>(io.BackendRendererUserData);
	io.BackendRendererUserData = nullptr;
}

ImTextureID c3d::ImGui_ImplVKObject_ToTextureID(const std::shared_ptr<VKImage>& image)
{
	ImGui_ImplVKObject_BackendData& bd = *static_cast<ImGui_ImplVKObject_BackendData*>(ImGui::GetIO().BackendRendererUserData);

	bd.referencedImages.emplace_back(image);

	return bd.referencedImages.size();
}