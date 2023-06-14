#include "SkyboxPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Asset/BindlessTextureManager.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"

#include <glm/gtx/transform.hpp>

SkyboxPass::SkyboxPass(glm::uvec2 size):
	RenderPass(size, "Skybox pass")
{
	createPipelineLayout();
	createPipeline();
	createImages();
	createBuffer();
	createSampler();
}

SkyboxPassOutput SkyboxPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, SkyboxPassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		_resolvedRawRenderImage.getCurrent(),
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal);
	
	vk::RenderingAttachmentInfo colorAttachment;
	colorAttachment.imageView = input.multisampledRawRenderImageView->getHandle();
	colorAttachment.imageLayout = input.multisampledRawRenderImageView->getInfo().getImage()->getLayout(0, 0);
	colorAttachment.resolveMode = vk::ResolveModeFlagBits::eAverage;
	colorAttachment.resolveImageView = _resolvedRawRenderImageView->getHandle();
	colorAttachment.resolveImageLayout = _resolvedRawRenderImage->getLayout(0, 0);
	colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.clearValue.color.float32[0] = 0.0f;
	colorAttachment.clearValue.color.float32[1] = 0.0f;
	colorAttachment.clearValue.color.float32[2] = 0.0f;
	colorAttachment.clearValue.color.float32[3] = 1.0f;
	
	vk::RenderingAttachmentInfo depthAttachment;
	depthAttachment.imageView = input.multisampledDepthImageView->getHandle();
	depthAttachment.imageLayout = input.multisampledDepthImageView->getInfo().getImage()->getLayout(0, 0);
	depthAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	depthAttachment.resolveImageView = nullptr;
	depthAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eNone;
	depthAttachment.clearValue.depthStencil.depth = 1.0f;
	
	vk::RenderingInfo renderingInfo;
	renderingInfo.renderArea.offset = vk::Offset2D(0, 0);
	renderingInfo.renderArea.extent = vk::Extent2D(_size.x, _size.y);
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = &depthAttachment;
	renderingInfo.pStencilAttachment = nullptr;
	
	commandBuffer->beginRendering(renderingInfo);
	
	if (Engine::getScene().getSkybox() != nullptr && Engine::getScene().getSkybox()->isLoaded())
	{
		commandBuffer->bindPipeline(_pipeline);
		
		VKPipelineViewport viewport;
		viewport.offset = {0, 0};
		viewport.size = _size;
		viewport.depthRange = {0.0f, 1.0f};
		commandBuffer->setViewport(viewport);
		
		VKPipelineScissor scissor;
		scissor.offset = {0, 0};
		scissor.size = _size;
		commandBuffer->setScissor(scissor);
		
		commandBuffer->bindDescriptorSet(0, Engine::getAssetManager().getBindlessTextureManager().getDescriptorSet());
		
		PushConstantData pushConstantData{};
		pushConstantData.mvp = input.camera.getProjection() *
		                       glm::mat4(glm::mat3(input.camera.getView())) *
		                       glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3(0, 1, 0));
		pushConstantData.textureIndex = Engine::getScene().getSkybox()->getBindlessIndex();
		commandBuffer->pushConstants(pushConstantData);
		
		commandBuffer->bindVertexBuffer(0, _vertexBuffer);
		
		commandBuffer->draw(_vertexBuffer->getSize(), 0);
		
		commandBuffer->unbindPipeline();
	}
	
	commandBuffer->endRendering();
	
	return {
		.rawRenderImageView = _resolvedRawRenderImageView.getCurrent()
	};
}

void SkyboxPass::onResize()
{
	createImages();
}

void SkyboxPass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(Engine::getAssetManager().getBindlessTextureManager().getDescriptorSetLayout());
	info.setPushConstantLayout<PushConstantData>();
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void SkyboxPass::createPipeline()
{
	VKGraphicsPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/skybox/skybox.vert",
		vk::PrimitiveTopology::eTriangleList,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise);
	
	info.setFragmentShader("resources/shaders/internal/skybox/skybox.frag");
	
	info.getVertexInputLayoutInfo().defineSlot(0, sizeof(SkyboxPass::VertexData), vk::VertexInputRate::eVertex);
	info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(SkyboxPass::VertexData, position));
	
	info.setRasterizationSampleCount(vk::SampleCountFlagBits::e4);
	
	info.getPipelineAttachmentInfo().addColorAttachment(SceneRenderer::HDR_COLOR_FORMAT);
	info.getPipelineAttachmentInfo().setDepthAttachment(SceneRenderer::DEPTH_FORMAT, vk::CompareOp::eLessOrEqual, false);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void SkyboxPass::createImages()
{
	{
		VKImageInfo imageInfo(
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_resolvedRawRenderImage = VKDynamic<VKImage>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKImage::create(context, imageInfo);
		});
		
		_resolvedRawRenderImageView = VKDynamic<VKImageView>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			VKImageViewInfo imageViewInfo(
				_resolvedRawRenderImage[index],
				vk::ImageViewType::e2D);
			
			return VKImageView::create(context, imageViewInfo);
		});
	}
}

void SkyboxPass::createBuffer()
{
	std::vector<SkyboxPass::VertexData> vertices = {
		{{-1.0f,  1.0f, -1.0f}},
		{{-1.0f, -1.0f, -1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},
		{{ 1.0f,  1.0f, -1.0f}},
		{{-1.0f,  1.0f, -1.0f}},
		
		{{-1.0f, -1.0f,  1.0f}},
		{{-1.0f, -1.0f, -1.0f}},
		{{-1.0f,  1.0f, -1.0f}},
		{{-1.0f,  1.0f, -1.0f}},
		{{-1.0f,  1.0f,  1.0f}},
		{{-1.0f, -1.0f,  1.0f}},
		
		{{ 1.0f, -1.0f, -1.0f}},
		{{ 1.0f, -1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f, -1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},
		
		{{-1.0f, -1.0f,  1.0f}},
		{{-1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f, -1.0f,  1.0f}},
		{{-1.0f, -1.0f,  1.0f}},
		
		{{-1.0f,  1.0f, -1.0f}},
		{{ 1.0f,  1.0f, -1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{-1.0f,  1.0f,  1.0f}},
		{{-1.0f,  1.0f, -1.0f}},
		
		{{-1.0f, -1.0f, -1.0f}},
		{{-1.0f, -1.0f,  1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},
		{{-1.0f, -1.0f,  1.0f}},
		{{ 1.0f, -1.0f,  1.0f}}
	};
	
	_vertexBuffer = VKBuffer<VertexData>::create(
		Engine::getVKContext(),
		vertices.size(),
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	VertexData* vertexBufferPtr = _vertexBuffer->map();
	std::copy(vertices.begin(), vertices.end(), vertexBufferPtr);
	_vertexBuffer->unmap();
}

void SkyboxPass::createSampler()
{
	vk::SamplerCreateInfo createInfo;
	createInfo.flags = {};
	createInfo.magFilter = vk::Filter::eLinear;
	createInfo.minFilter = vk::Filter::eLinear;
	createInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	createInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
	createInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
	createInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
	createInfo.mipLodBias = 0.0f;
	createInfo.anisotropyEnable = true;
	createInfo.maxAnisotropy = 16;
	createInfo.compareEnable = false;
	createInfo.compareOp = vk::CompareOp::eNever;
	createInfo.minLod = -1000.0f;
	createInfo.maxLod = 1000.0f;
	createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	createInfo.unnormalizedCoordinates = false;
	
	_sampler = VKSampler::create(Engine::getVKContext(), createInfo);
}