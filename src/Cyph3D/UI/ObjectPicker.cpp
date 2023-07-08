#include "ObjectPicker.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Entity/Component/ModelRenderer.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/VertexData.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayoutInfo.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayoutInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"

ObjectPicker::ObjectPicker()
{
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createBuffer();
}

ObjectPicker::~ObjectPicker()
{

}

Entity* ObjectPicker::getPickedEntity(Camera& camera, const RenderRegistry& renderRegistry, const glm::uvec2& viewportSize, const glm::uvec2& clickPos)
{
	if (viewportSize.x * viewportSize.y == 0)
		return nullptr;
	
	if (viewportSize != _currentSize)
	{
		_currentSize = viewportSize;
		createImage();
	}
	
	Engine::getVKContext().executeImmediate(
		[&](const VKPtr<VKCommandBuffer>& commandBuffer)
		{
			commandBuffer->imageMemoryBarrier(
				_objectIndexImage,
				0,
				0,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::ImageLayout::eColorAttachmentOptimal);
			
			commandBuffer->imageMemoryBarrier(
				_depthImage,
				0,
				0,
				vk::PipelineStageFlagBits2::eNone,
				vk::AccessFlagBits2::eNone,
				vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
				vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
				vk::ImageLayout::eDepthAttachmentOptimal);
			
			VKRenderingInfo renderingInfo(_currentSize);
			
			renderingInfo.addColorAttachment(_objectIndexImageView)
				.setLoadOpClear(glm::ivec4(-1, 0, 0, 0))
				.setStoreOpStore();
			
			renderingInfo.setDepthAttachment(_depthImageView)
				.setLoadOpClear(1.0f)
				.setStoreOpStore();
			
			commandBuffer->beginRendering(renderingInfo);
			
			commandBuffer->bindPipeline(_pipeline);
			
			VKPipelineViewport viewport;
			viewport.offset = {0, 0};
			viewport.size = _currentSize;
			viewport.depthRange = {0.0f, 1.0f};
			commandBuffer->setViewport(viewport);
			
			VKPipelineScissor scissor;
			scissor.offset = {0, 0};
			scissor.size = _currentSize;
			commandBuffer->setScissor(scissor);
			
			glm::mat4 vp = camera.getProjection() * camera.getView();
			
			for (int i = 0; i < renderRegistry.getModelRenderRequests().size(); i++)
			{
				const ModelRenderer::RenderData& renderData = renderRegistry.getModelRenderRequests()[i];
				
				const VKPtr<VKBuffer<VertexData>>& vertexBuffer = renderData.mesh->getVertexBuffer();
				const VKPtr<VKBuffer<uint32_t>>& indexBuffer = renderData.mesh->getIndexBuffer();
				
				commandBuffer->bindVertexBuffer(0, vertexBuffer);
				commandBuffer->bindIndexBuffer(indexBuffer);
				
				PushConstantData pushConstantData{};
				pushConstantData.mvp = vp * renderData.matrix;
				pushConstantData.objectIndex = i;
				commandBuffer->pushConstants(pushConstantData);
				
				commandBuffer->draw(indexBuffer->getSize(), 0);
			}
			
			commandBuffer->unbindPipeline();
			
			commandBuffer->endRendering();
			
			commandBuffer->imageMemoryBarrier(
				_objectIndexImage,
				0,
				0,
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,
				vk::AccessFlagBits2::eColorAttachmentWrite,
				vk::PipelineStageFlagBits2::eCopy,
				vk::AccessFlagBits2::eTransferRead,
				vk::ImageLayout::eTransferSrcOptimal);
			
			commandBuffer->copyPixelToBuffer(_objectIndexImage, 0, 0, clickPos, _readbackBuffer, 0);
		});
	
	int objectIndex = *_readbackBuffer->getHostPointer();
	
	return objectIndex != -1 ? renderRegistry.getModelRenderRequests()[objectIndex].owner : nullptr;
}

void ObjectPicker::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eStorageBuffer, 1);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void ObjectPicker::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(_descriptorSetLayout);
	info.setPushConstantLayout<PushConstantData>();
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void ObjectPicker::createPipeline()
{
	VKGraphicsPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/object picker/object picker.vert",
		vk::PrimitiveTopology::eTriangleList,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise);
	
	info.setFragmentShader("resources/shaders/internal/object picker/object picker.frag");
	
	info.getVertexInputLayoutInfo().defineSlot(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
	info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, position));
	
	info.getPipelineAttachmentInfo().addColorAttachment(vk::Format::eR32Sint);
	info.getPipelineAttachmentInfo().setDepthAttachment(vk::Format::eD32Sfloat, vk::CompareOp::eLess, true);
	
	_pipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
}

void ObjectPicker::createBuffer()
{
	VKBufferInfo bufferInfo(1, vk::BufferUsageFlagBits::eTransferDst);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCached);
	
	_readbackBuffer = VKBuffer<int32_t>::create(Engine::getVKContext(), bufferInfo);
}

void ObjectPicker::createImage()
{
	{
		VKImageInfo imageInfo(
			vk::Format::eR32Sint,
			_currentSize,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_objectIndexImage = VKImage::create(Engine::getVKContext(), imageInfo);
		
		VKImageViewInfo imageViewInfo(
			_objectIndexImage,
			vk::ImageViewType::e2D);
		
		_objectIndexImageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
	}
	
	{
		VKImageInfo imageInfo(
			vk::Format::eD32Sfloat,
			_currentSize,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_depthImage = VKImage::create(Engine::getVKContext(), imageInfo);
		
		VKImageViewInfo imageViewInfo(
			_depthImage,
			vk::ImageViewType::e2D);
		
		_depthImageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
	}
}