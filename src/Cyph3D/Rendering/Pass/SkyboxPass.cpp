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
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"

#include <glm/gtx/transform.hpp>

SkyboxPass::SkyboxPass(glm::uvec2 size):
	RenderPass(size, "Skybox pass")
{
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createBuffer();
	createSampler();
}

SkyboxPassOutput SkyboxPass::renderImpl(const VKPtr<VKCommandBuffer>& commandBuffer, SkyboxPassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		input.rawRenderView->getImage(),
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentRead,
		vk::ImageLayout::eColorAttachmentOptimal,
		0,
		0);
	
	vk::RenderingAttachmentInfo colorAttachment;
	colorAttachment.imageView = input.rawRenderView->getHandle();
	colorAttachment.imageLayout = input.rawRenderView->getImage()->getLayout(0, 0);
	colorAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
	colorAttachment.resolveImageView = nullptr;
	colorAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.clearValue.color.float32[0] = 0.0f;
	colorAttachment.clearValue.color.float32[1] = 0.0f;
	colorAttachment.clearValue.color.float32[2] = 0.0f;
	colorAttachment.clearValue.color.float32[3] = 1.0f;
	
	vk::RenderingAttachmentInfo depthAttachment;
	depthAttachment.imageView = input.depthView->getHandle();
	depthAttachment.imageLayout = input.depthView->getImage()->getLayout(0, 0);
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
	
	commandBuffer->bindPipeline(_pipeline);
	
	commandBuffer->pushDescriptor(0, 0, Engine::getScene().getSkybox()->getImageView(), _sampler);
	
	PushConstantData pushConstantData{};
	pushConstantData.mvp = input.camera.getProjection() *
						   glm::mat4(glm::mat3(input.camera.getView())) *
						   glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3(0, 1, 0));
	commandBuffer->pushConstants(vk::ShaderStageFlagBits::eVertex, pushConstantData);
	
	commandBuffer->bindVertexBuffer(0, _vertexBuffer);
	
	commandBuffer->draw(_vertexBuffer->getSize(), 0);
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	return {};
}

void SkyboxPass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.registerBinding(0, vk::DescriptorType::eCombinedImageSampler, 1);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void SkyboxPass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.registerDescriptorSetLayout(_descriptorSetLayout);
	info.registerPushConstantLayout<PushConstantData>(vk::ShaderStageFlagBits::eVertex);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void SkyboxPass::createPipeline()
{
	VKGraphicsPipelineInfo info;
	info.vertexShaderFile = "resources/shaders/internal/skybox/skybox.vert";
	info.geometryShaderFile = std::nullopt;
	info.fragmentShaderFile = "resources/shaders/internal/skybox/skybox.frag";
	
	info.vertexInputLayoutInfo.defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(SkyboxPass::VertexData, position));
	info.vertexInputLayoutInfo.defineSlot(0, sizeof(SkyboxPass::VertexData), vk::VertexInputRate::eVertex);
	
	info.vertexTopology = vk::PrimitiveTopology::eTriangleList;
	
	info.pipelineLayout = _pipelineLayout;
	
	info.viewport = VKPipelineViewport{
		.offset = {0, 0},
		.size = _size,
		.depthRange = {0.0f, 1.0f}
	};
	
	info.scissor = VKPipelineScissor{
		.offset = info.viewport->offset,
		.size = info.viewport->size
	};
	
	info.rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
	info.rasterizationInfo.frontFace = vk::FrontFace::eCounterClockwise;
	
	info.pipelineAttachmentInfo.registerColorAttachment(0, SceneRenderer::HDR_COLOR_FORMAT);
	info.pipelineAttachmentInfo.setDepthAttachment(SceneRenderer::DEPTH_FORMAT, vk::CompareOp::eLessOrEqual, false);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
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
	createInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
	createInfo.unnormalizedCoordinates = false;
	
	_sampler = VKSampler::create(Engine::getVKContext(), createInfo);
}
