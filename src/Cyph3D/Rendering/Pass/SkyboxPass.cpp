#include "SkyboxPass.h"

#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Asset/BindlessTextureManager.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"

#include <glm/gtx/transform.hpp>

SkyboxPass::SkyboxPass(glm::uvec2 size):
	RenderPass(size, "Skybox pass")
{
	createPipelineLayout();
	createPipeline();
	createImages();
	createBuffer();
}

SkyboxPassOutput SkyboxPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, SkyboxPassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		_resolvedRawRenderImage,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	VKRenderingInfo renderingInfo(_size);

	renderingInfo.addColorAttachment(input.multisampledRawRenderImage)
		.enableResolve(vk::ResolveModeFlagBits::eAverage, _resolvedRawRenderImage)
		.setLoadOpLoad()
		.setStoreOpStore();

	renderingInfo.setDepthAttachment(input.multisampledDepthImage)
		.setLoadOpLoad()
		.setStoreOpNone();

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
		pushConstantData.textureIndex = Engine::getScene().getSkybox()->getCubemap()->getBindlessIndex();
		commandBuffer->pushConstants(pushConstantData);

		commandBuffer->bindVertexBuffer(0, _vertexBuffer);

		commandBuffer->draw(_vertexBuffer->getSize(), 0);

		commandBuffer->unbindPipeline();
	}

	commandBuffer->endRendering();

	return {
		.rawRenderImage = _resolvedRawRenderImage
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
		vk::FrontFace::eCounterClockwise
	);

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
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
		);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		imageInfo.setName("Resolved raw render image");

		_resolvedRawRenderImage = VKImage::create(Engine::getVKContext(), imageInfo);
	}
}

void SkyboxPass::createBuffer()
{
	std::vector<SkyboxPass::VertexData> vertices = {
		{{-1.0f, 1.0f, -1.0f}},
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, -1.0f, -1.0f}},
		{{1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, -1.0f}},
		{{-1.0f, 1.0f, -1.0f}},

		{{-1.0f, -1.0f, 1.0f}},
		{{-1.0f, -1.0f, -1.0f}},
		{{-1.0f, 1.0f, -1.0f}},
		{{-1.0f, 1.0f, -1.0f}},
		{{-1.0f, 1.0f, 1.0f}},
		{{-1.0f, -1.0f, 1.0f}},

		{{1.0f, -1.0f, -1.0f}},
		{{1.0f, -1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f}},
		{{1.0f, -1.0f, -1.0f}},

		{{-1.0f, -1.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f}},
		{{-1.0f, -1.0f, 1.0f}},

		{{-1.0f, 1.0f, -1.0f}},
		{{1.0f, 1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, 1.0f}},
		{{-1.0f, 1.0f, -1.0f}},

		{{-1.0f, -1.0f, -1.0f}},
		{{-1.0f, -1.0f, 1.0f}},
		{{1.0f, -1.0f, -1.0f}},
		{{1.0f, -1.0f, -1.0f}},
		{{-1.0f, -1.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f}}
	};

	VKBufferInfo vertexBufferInfo(vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);
	vertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	vertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	vertexBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	vertexBufferInfo.setName("Skybox vertex buffer");

	_vertexBuffer = VKBuffer<VertexData>::create(Engine::getVKContext(), vertexBufferInfo);

	std::copy(vertices.begin(), vertices.end(), _vertexBuffer->getHostPointer());
}