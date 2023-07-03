#include "VKCommandBuffer.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/VKHelper.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKBottomLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKTopLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Buffer/VKBufferBase.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineViewport.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineScissor.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/VKObject/Fence/VKFence.h"
#include "Cyph3D/VKObject/Semaphore/VKSemaphore.h"
#include "Cyph3D/VKObject/Query/VKAccelerationStructureCompactedSizeQuery.h"
#include "Cyph3D/VKObject/Query/VKTimestampQuery.h"

VKPtr<VKCommandBuffer> VKCommandBuffer::create(VKContext& context, const VKQueue& queue)
{
	return VKPtr<VKCommandBuffer>(new VKCommandBuffer(context, queue));
}

VKCommandBuffer::VKCommandBuffer(VKContext& context, const VKQueue& queue):
	VKObject(context)
{
	vk::CommandPoolCreateInfo poolCreateInfo;
	poolCreateInfo.queueFamilyIndex = queue.getFamily();
	
	_commandPool = _context.getDevice().createCommandPool(poolCreateInfo);
	
	vk::CommandBufferAllocateInfo allocateInfo;
	allocateInfo.commandPool = _commandPool;
	allocateInfo.level = vk::CommandBufferLevel::ePrimary;
	allocateInfo.commandBufferCount = 1;
	
	_commandBuffer = _context.getDevice().allocateCommandBuffers(allocateInfo).front();
	
	vk::FenceCreateInfo fenceCreateInfo;
	fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
	
	_statusFence = VKFence::create(_context, fenceCreateInfo);
	
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	
	_statusSemaphore = VKSemaphore::create(_context, semaphoreCreateInfo);
}

VKCommandBuffer::~VKCommandBuffer()
{
	_context.getDevice().destroyCommandPool(_commandPool);
}

const vk::CommandBuffer& VKCommandBuffer::getHandle()
{
	return _commandBuffer;
}

bool VKCommandBuffer::waitExecution() const
{
	return _statusFence->wait();
}

void VKCommandBuffer::begin()
{
	waitExecution();
	
	reset();
	
	vk::CommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional
	
	_commandBuffer.begin(commandBufferBeginInfo);
}

void VKCommandBuffer::end()
{
	_commandBuffer.end();
}

void VKCommandBuffer::reset()
{
	_context.getDevice().resetCommandPool(_commandPool);
	_usedObjects.clear();
	_boundPipeline = nullptr;
}

void VKCommandBuffer::memoryBarrier(vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask)
{
	vk::MemoryBarrier2 memoryBarrier;
	memoryBarrier.srcStageMask = srcStageMask;
	memoryBarrier.srcAccessMask = srcAccessMask;
	memoryBarrier.dstStageMask = dstStageMask;
	memoryBarrier.dstAccessMask = dstAccessMask;
	
	vk::DependencyInfo dependencyInfo;
	dependencyInfo.memoryBarrierCount = 1;
	dependencyInfo.pMemoryBarriers = &memoryBarrier;
	
	_commandBuffer.pipelineBarrier2(dependencyInfo);
}

void VKCommandBuffer::bufferMemoryBarrier(const VKPtr<VKBufferBase>& buffer, vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask)
{
	vk::BufferMemoryBarrier2 bufferMemoryBarrier;
	bufferMemoryBarrier.srcStageMask = srcStageMask;
	bufferMemoryBarrier.srcAccessMask = srcAccessMask;
	bufferMemoryBarrier.dstStageMask = dstStageMask;
	bufferMemoryBarrier.dstAccessMask = dstAccessMask;
	bufferMemoryBarrier.buffer = buffer->getHandle();
	bufferMemoryBarrier.offset = 0;
	bufferMemoryBarrier.size = VK_WHOLE_SIZE;
	
	vk::DependencyInfo dependencyInfo;
	dependencyInfo.bufferMemoryBarrierCount = 1;
	dependencyInfo.pBufferMemoryBarriers = &bufferMemoryBarrier;
	
	_commandBuffer.pipelineBarrier2(dependencyInfo);
	
	_usedObjects.emplace_back(buffer);
}

void VKCommandBuffer::imageMemoryBarrier(const VKPtr<VKImage>& image, uint32_t layer, uint32_t level, vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask)
{
	vk::ImageMemoryBarrier2 imageMemoryBarrier;
	imageMemoryBarrier.srcStageMask = srcStageMask;
	imageMemoryBarrier.srcAccessMask = srcAccessMask;
	imageMemoryBarrier.dstStageMask = dstStageMask;
	imageMemoryBarrier.dstAccessMask = dstAccessMask;
	imageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
	imageMemoryBarrier.newLayout = vk::ImageLayout::eUndefined;
	imageMemoryBarrier.image = image->getHandle();
	imageMemoryBarrier.subresourceRange.aspectMask = VKHelper::getAspect(image->getInfo().getFormat());
	imageMemoryBarrier.subresourceRange.baseMipLevel = level;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = layer;
	imageMemoryBarrier.subresourceRange.layerCount = 1;
	
	vk::DependencyInfo dependencyInfo;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;
	
	_commandBuffer.pipelineBarrier2(dependencyInfo);
	
	_usedObjects.emplace_back(image);
}

void VKCommandBuffer::imageMemoryBarrier(const VKPtr<VKImage>& image, uint32_t layer, uint32_t level, vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask, vk::ImageLayout newImageLayout)
{
	vk::ImageMemoryBarrier2 imageMemoryBarrier;
	imageMemoryBarrier.srcStageMask = srcStageMask;
	imageMemoryBarrier.srcAccessMask = srcAccessMask;
	imageMemoryBarrier.dstStageMask = dstStageMask;
	imageMemoryBarrier.dstAccessMask = dstAccessMask;
	imageMemoryBarrier.oldLayout = image->getLayout(layer, level);
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image->getHandle();
	imageMemoryBarrier.subresourceRange.aspectMask = VKHelper::getAspect(image->getInfo().getFormat());
	imageMemoryBarrier.subresourceRange.baseMipLevel = level;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = layer;
	imageMemoryBarrier.subresourceRange.layerCount = 1;
	
	vk::DependencyInfo dependencyInfo;
	dependencyInfo.imageMemoryBarrierCount = 1;
	dependencyInfo.pImageMemoryBarriers = &imageMemoryBarrier;
	
	_commandBuffer.pipelineBarrier2(dependencyInfo);
	
	image->setLayout(layer, level, newImageLayout);
	
	_usedObjects.emplace_back(image);
}

void VKCommandBuffer::beginRendering(const VKRenderingInfo& renderingInfo)
{
	if (_boundPipeline != nullptr)
		throw;
	
	std::vector<vk::RenderingAttachmentInfo> colorAttachments;
	colorAttachments.reserve(renderingInfo.getColorAttachmentInfos().size());
	for (const VKRenderingColorAttachmentInfo& colorAttachmentInfo : renderingInfo.getColorAttachmentInfos())
	{
		const VKPtr<VKImageView>& imageView = colorAttachmentInfo.getImageView();
		
		_usedObjects.emplace_back(imageView);

		VKHelper::assertImageViewHasUniqueLayout(imageView);
		
		vk::RenderingAttachmentInfo& colorAttachment = colorAttachments.emplace_back();
		colorAttachment.imageView = imageView->getHandle();
		colorAttachment.imageLayout = imageView->getInfo().getImage()->getLayout(imageView->getFirstReferencedLayer(), imageView->getFirstReferencedLevel());
		
		if (colorAttachmentInfo.getResolveMode() != vk::ResolveModeFlagBits::eNone)
		{
			colorAttachment.resolveMode = colorAttachmentInfo.getResolveMode();
			
			const VKPtr<VKImageView>& resolveImageView = colorAttachmentInfo.getResolveImageView();
			
			VKHelper::assertImageViewHasUniqueLayout(resolveImageView);
			
			colorAttachment.resolveImageView = resolveImageView->getHandle();
			colorAttachment.resolveImageLayout = resolveImageView->getInfo().getImage()->getLayout(resolveImageView->getFirstReferencedLayer(), resolveImageView->getFirstReferencedLevel());
		}
		else
		{
			colorAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
			colorAttachment.resolveImageView = nullptr;
			colorAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
		}
		
		colorAttachment.loadOp = colorAttachmentInfo.getLoadOp();
		colorAttachment.storeOp = colorAttachmentInfo.getStoreOp();
		colorAttachment.clearValue = colorAttachmentInfo.getClearValue();
	}
	
	vk::RenderingInfo vkRenderingInfo;
	vkRenderingInfo.renderArea.offset = vk::Offset2D(0, 0);
	vkRenderingInfo.renderArea.extent = vk::Extent2D(renderingInfo.getSize().x, renderingInfo.getSize().y);
	vkRenderingInfo.layerCount = renderingInfo.getLayers();
	vkRenderingInfo.viewMask = 0;
	vkRenderingInfo.colorAttachmentCount = colorAttachments.size();
	vkRenderingInfo.pColorAttachments = colorAttachments.data();
	vkRenderingInfo.pDepthAttachment = nullptr;
	
	vk::RenderingAttachmentInfo depthAttachment;
	if (renderingInfo.hasDepthAttachment())
	{
		const VKRenderingDepthAttachmentInfo& depthAttachmentInfo = renderingInfo.getDepthAttachmentInfo();
		
		const VKPtr<VKImageView>& imageView = depthAttachmentInfo.getImageView();
		
		_usedObjects.emplace_back(imageView);
		
		VKHelper::assertImageViewHasUniqueLayout(imageView);
		
		depthAttachment.imageView = imageView->getHandle();
		depthAttachment.imageLayout = imageView->getInfo().getImage()->getLayout(imageView->getFirstReferencedLayer(), imageView->getFirstReferencedLevel());
		
		if (depthAttachmentInfo.getResolveMode() != vk::ResolveModeFlagBits::eNone)
		{
			depthAttachment.resolveMode = depthAttachmentInfo.getResolveMode();
			
			const VKPtr<VKImageView>& resolveImageView = depthAttachmentInfo.getResolveImageView();
			
			VKHelper::assertImageViewHasUniqueLayout(resolveImageView);
			
			depthAttachment.resolveImageView = resolveImageView->getHandle();
			depthAttachment.resolveImageLayout = resolveImageView->getInfo().getImage()->getLayout(resolveImageView->getFirstReferencedLayer(), resolveImageView->getFirstReferencedLevel());
		}
		else
		{
			depthAttachment.resolveMode = vk::ResolveModeFlagBits::eNone;
			depthAttachment.resolveImageView = nullptr;
			depthAttachment.resolveImageLayout = vk::ImageLayout::eUndefined;
		}
		
		depthAttachment.loadOp = depthAttachmentInfo.getLoadOp();
		depthAttachment.storeOp = depthAttachmentInfo.getStoreOp();
		depthAttachment.clearValue = depthAttachmentInfo.getClearValue();
		
		
		vkRenderingInfo.pDepthAttachment = &depthAttachment;
	}
	else
	{
		vkRenderingInfo.pDepthAttachment = nullptr;
	}
	
	vkRenderingInfo.pStencilAttachment = nullptr;
	
	_commandBuffer.beginRendering(vkRenderingInfo);
}

void VKCommandBuffer::endRendering()
{
	if (_boundPipeline != nullptr)
		throw;
	
	_commandBuffer.endRendering();
}

void VKCommandBuffer::bindPipeline(const VKPtr<VKPipeline>& pipeline)
{
	if (_boundPipeline != nullptr)
		throw;
	
	_commandBuffer.bindPipeline(pipeline->getPipelineType(), pipeline->getHandle());
	_boundPipeline = pipeline.get();
	
	_usedObjects.emplace_back(pipeline);
}

void VKCommandBuffer::unbindPipeline()
{
	if (_boundPipeline == nullptr)
		throw;
	
	_boundPipeline = nullptr;
}

void VKCommandBuffer::bindDescriptorSet(uint32_t setIndex, const VKPtr<VKDescriptorSet>& descriptorSet)
{
	if (_boundPipeline == nullptr)
		throw;
	
	_commandBuffer.bindDescriptorSets(
		_boundPipeline->getPipelineType(),
		_boundPipeline->getPipelineLayout()->getHandle(),
		setIndex,
		descriptorSet->getHandle(),
		{});
	
	_usedObjects.emplace_back(descriptorSet);
}

void VKCommandBuffer::bindDescriptorSet(uint32_t setIndex, const VKPtr<VKDescriptorSet>& descriptorSet, uint32_t dynamicOffset)
{
	if (_boundPipeline == nullptr)
		throw;
	
	_commandBuffer.bindDescriptorSets(
		_boundPipeline->getPipelineType(),
		_boundPipeline->getPipelineLayout()->getHandle(),
		setIndex,
		descriptorSet->getHandle(),
		dynamicOffset);
	
	_usedObjects.emplace_back(descriptorSet);
}

void VKCommandBuffer::pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKBufferBase>& buffer, size_t offset, size_t size, uint32_t arrayIndex)
{
	if (_boundPipeline == nullptr)
		throw;
	
	vk::DescriptorBufferInfo bufferInfo;
	if (size > 0)
	{
		bufferInfo.buffer = buffer->getHandle();
		bufferInfo.offset = offset * buffer->getStride();
		bufferInfo.range = size * buffer->getStride();
		
		_usedObjects.emplace_back(buffer);
	}
	else
	{
		bufferInfo.buffer = VK_NULL_HANDLE;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;
	}
	
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _boundPipeline->getPipelineLayout()->getInfo().getDescriptorSetLayout(setIndex)->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = VK_NULL_HANDLE;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pTexelBufferView = nullptr; // Optional
	
	_commandBuffer.pushDescriptorSetKHR(
		_boundPipeline->getPipelineType(),
		_boundPipeline->getPipelineLayout()->getHandle(),
		setIndex,
		descriptorWrite);
	
	_usedObjects.emplace_back(buffer);
}

void VKCommandBuffer::pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex)
{
	if (_boundPipeline == nullptr)
		throw;
	
	vk::DescriptorImageInfo samplerInfo;
	samplerInfo.sampler = sampler->getHandle();
	
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _boundPipeline->getPipelineLayout()->getInfo().getDescriptorSetLayout(setIndex)->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = VK_NULL_HANDLE;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &samplerInfo;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	
	_commandBuffer.pushDescriptorSetKHR(
		_boundPipeline->getPipelineType(),
		_boundPipeline->getPipelineLayout()->getHandle(),
		setIndex,
		descriptorWrite);
	
	_usedObjects.emplace_back(sampler);
}

void VKCommandBuffer::pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKImageView>& imageView, uint32_t arrayIndex)
{
	if (_boundPipeline == nullptr)
		throw;
	
	VKHelper::assertImageViewHasUniqueLayout(imageView);
	
	vk::DescriptorImageInfo imageInfo;
	imageInfo.imageView = imageView->getHandle();
	imageInfo.imageLayout = imageView->getInfo().getImage()->getLayout(imageView->getFirstReferencedLayer(), imageView->getFirstReferencedLevel());
	
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _boundPipeline->getPipelineLayout()->getInfo().getDescriptorSetLayout(setIndex)->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = VK_NULL_HANDLE;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	
	_commandBuffer.pushDescriptorSetKHR(
		_boundPipeline->getPipelineType(),
		_boundPipeline->getPipelineLayout()->getHandle(),
		setIndex,
		descriptorWrite);
	
	_usedObjects.emplace_back(imageView);
}

void VKCommandBuffer::pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKImageView>& imageView, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex)
{
	if (_boundPipeline == nullptr)
		throw;
	
	VKHelper::assertImageViewHasUniqueLayout(imageView);
	
	vk::DescriptorImageInfo combinedImageSamplerInfo;
	combinedImageSamplerInfo.imageView = imageView->getHandle();
	combinedImageSamplerInfo.imageLayout = imageView->getInfo().getImage()->getLayout(imageView->getFirstReferencedLayer(), imageView->getFirstReferencedLevel());
	combinedImageSamplerInfo.sampler = sampler->getHandle();
	
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _boundPipeline->getPipelineLayout()->getInfo().getDescriptorSetLayout(setIndex)->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = VK_NULL_HANDLE;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &combinedImageSamplerInfo;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	
	_commandBuffer.pushDescriptorSetKHR(
		_boundPipeline->getPipelineType(),
		_boundPipeline->getPipelineLayout()->getHandle(),
		setIndex,
		descriptorWrite);
	
	_usedObjects.emplace_back(imageView);
	_usedObjects.emplace_back(sampler);
}

void VKCommandBuffer::pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKAccelerationStructure>& accelerationStructure, uint32_t arrayIndex)
{
	if (_boundPipeline == nullptr)
		throw;
	
	const VKDescriptorSetLayoutInfo::BindingInfo& bindingInfo = _boundPipeline->getPipelineLayout()->getInfo().getDescriptorSetLayout(setIndex)->getInfo().getBindingInfo(bindingIndex);
	
	vk::WriteDescriptorSetAccelerationStructureKHR accelerationStructureDescriptorWrite;
	accelerationStructureDescriptorWrite.accelerationStructureCount = 1;
	accelerationStructureDescriptorWrite.pAccelerationStructures = &accelerationStructure->getHandle();
	
	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = VK_NULL_HANDLE;
	descriptorWrite.dstBinding = bindingIndex;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = bindingInfo.type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;
	descriptorWrite.pNext = &accelerationStructureDescriptorWrite;
	
	_commandBuffer.pushDescriptorSetKHR(
		_boundPipeline->getPipelineType(),
		_boundPipeline->getPipelineLayout()->getHandle(),
		setIndex,
		descriptorWrite);
	
	_usedObjects.emplace_back(accelerationStructure);
}

void VKCommandBuffer::bindVertexBuffer(uint32_t vertexBufferIndex, const VKPtr<VKBufferBase>& vertexBuffer)
{
	if (_boundPipeline == nullptr)
		throw;
	
	_commandBuffer.bindVertexBuffers(vertexBufferIndex, vertexBuffer->getHandle(), {0});
	
	_usedObjects.emplace_back(vertexBuffer);
}

void VKCommandBuffer::bindIndexBuffer(const VKPtr<VKBufferBase>& indexBuffer)
{
	if (_boundPipeline == nullptr)
		throw;
	
	vk::IndexType indexType;
	switch (indexBuffer->getStride())
	{
		case 2:
			indexType = vk::IndexType::eUint16;
			break;
		case 4:
			indexType = vk::IndexType::eUint32;
			break;
		default:
			throw;
	}
	
	_commandBuffer.bindIndexBuffer(indexBuffer->getHandle(), 0, indexType);
	
	_usedObjects.emplace_back(indexBuffer);
}

void VKCommandBuffer::draw(uint32_t vertexCount, uint32_t vertexOffset)
{
	_commandBuffer.draw(vertexCount, 1, vertexOffset, 0);
}

void VKCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset)
{
	_commandBuffer.drawIndexed(indexCount, 1, indexOffset, vertexOffset, 0);
}

void VKCommandBuffer::copyBufferToImage(const VKPtr<VKBufferBase>& srcBuffer, vk::DeviceSize srcByteOffset, const VKPtr<VKImage>& dstImage, uint32_t dstLayer, uint32_t dstLevel)
{
	if (srcBuffer->getByteSize() - srcByteOffset < dstImage->getLevelByteSize(dstLevel))
		throw;
	
	glm::uvec2 dstSize = dstImage->getSize(dstLevel);
	
	vk::BufferImageCopy2 copiedRegion;
	copiedRegion.bufferOffset = srcByteOffset;
	copiedRegion.bufferRowLength = 0;
	copiedRegion.bufferImageHeight = 0;
	copiedRegion.imageSubresource.aspectMask = VKHelper::getAspect(dstImage->getInfo().getFormat());
	copiedRegion.imageSubresource.mipLevel = dstLevel;
	copiedRegion.imageSubresource.baseArrayLayer = dstLayer;
	copiedRegion.imageSubresource.layerCount = 1;
	copiedRegion.imageOffset = vk::Offset3D(0, 0, 0);
	copiedRegion.imageExtent = vk::Extent3D(dstSize.x, dstSize.y, 1);
	
	vk::CopyBufferToImageInfo2 copyInfo;
	copyInfo.srcBuffer = srcBuffer->getHandle();
	copyInfo.dstImage = dstImage->getHandle();
	copyInfo.dstImageLayout = dstImage->getLayout(dstLayer, dstLevel);
	copyInfo.regionCount = 1;
	copyInfo.pRegions = &copiedRegion;
	
	_commandBuffer.copyBufferToImage2(copyInfo);
	
	_usedObjects.emplace_back(srcBuffer);
	_usedObjects.emplace_back(dstImage);
}

void VKCommandBuffer::copyBufferToBuffer(const VKPtr<VKBufferBase>& srcBuffer, vk::DeviceSize srcByteOffset, const VKPtr<VKBufferBase>& dstBuffer, vk::DeviceSize dstByteOffset, vk::DeviceSize size)
{
	if (srcBuffer->getByteSize() - srcByteOffset < size || dstBuffer->getByteSize() - dstByteOffset < size)
		throw;
	
	vk::BufferCopy2 copiedRegion;
	copiedRegion.srcOffset = srcByteOffset;
	copiedRegion.dstOffset = dstByteOffset;
	copiedRegion.size = size;
	
	vk::CopyBufferInfo2 copyInfo;
	copyInfo.srcBuffer = srcBuffer->getHandle();
	copyInfo.dstBuffer = dstBuffer->getHandle();
	copyInfo.regionCount = 1;
	copyInfo.pRegions = &copiedRegion;
	
	_commandBuffer.copyBuffer2(copyInfo);
	
	_usedObjects.emplace_back(srcBuffer);
	_usedObjects.emplace_back(dstBuffer);
}

void VKCommandBuffer::copyImageToBuffer(const VKPtr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, const VKPtr<VKBufferBase>& dstBuffer, vk::DeviceSize dstByteOffset)
{
	if (srcImage->getLevelByteSize(srcLevel) > dstBuffer->getByteSize() - dstByteOffset)
		throw;
	
	glm::uvec2 srcSize = srcImage->getSize(srcLevel);
	
	vk::BufferImageCopy2 copiedRegion;
	copiedRegion.bufferOffset = dstByteOffset;
	copiedRegion.bufferRowLength = 0;
	copiedRegion.bufferImageHeight = 0;
	copiedRegion.imageSubresource.aspectMask = VKHelper::getAspect(srcImage->getInfo().getFormat());
	copiedRegion.imageSubresource.mipLevel = srcLevel;
	copiedRegion.imageSubresource.baseArrayLayer = srcLayer;
	copiedRegion.imageSubresource.layerCount = 1;
	copiedRegion.imageOffset = vk::Offset3D(0, 0, 0);
	copiedRegion.imageExtent = vk::Extent3D(srcSize.x, srcSize.y, 1);
	
	vk::CopyImageToBufferInfo2 copyInfo;
	copyInfo.srcImage = srcImage->getHandle();
	copyInfo.srcImageLayout = srcImage->getLayout(srcLayer, srcLevel);
	copyInfo.dstBuffer = dstBuffer->getHandle();
	copyInfo.regionCount = 1;
	copyInfo.pRegions = &copiedRegion;
	
	_commandBuffer.copyImageToBuffer2(copyInfo);
	
	_usedObjects.emplace_back(srcImage);
	_usedObjects.emplace_back(dstBuffer);
}

void VKCommandBuffer::copyImageToImage(const VKPtr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, const VKPtr<VKImage>& dstImage, uint32_t dstLayer, uint32_t dstLevel)
{
	if (srcImage->getLevelByteSize(srcLevel) != dstImage->getLevelByteSize(dstLevel))
		throw;
	
	glm::uvec2 srcSize = srcImage->getSize(srcLevel);
	glm::uvec2 dstSize = dstImage->getSize(dstLevel);
	
	if (srcSize != dstSize)
		throw;
	
	vk::ImageCopy2 copiedRegion;
	copiedRegion.srcSubresource.aspectMask = VKHelper::getAspect(srcImage->getInfo().getFormat());
	copiedRegion.srcSubresource.mipLevel = srcLevel;
	copiedRegion.srcSubresource.baseArrayLayer = srcLayer;
	copiedRegion.srcSubresource.layerCount = 1;
	copiedRegion.srcOffset = vk::Offset3D(0, 0, 0);
	copiedRegion.dstSubresource.aspectMask = VKHelper::getAspect(dstImage->getInfo().getFormat());
	copiedRegion.dstSubresource.mipLevel = dstLevel;
	copiedRegion.dstSubresource.baseArrayLayer = dstLayer;
	copiedRegion.dstSubresource.layerCount = 1;
	copiedRegion.dstOffset = vk::Offset3D(0, 0, 0);
	copiedRegion.extent = vk::Extent3D(srcSize.x, srcSize.y, 1);
	
	vk::CopyImageInfo2 copyInfo;
	copyInfo.srcImage = srcImage->getHandle();
	copyInfo.srcImageLayout = srcImage->getLayout(srcLayer, srcLevel);
	copyInfo.dstImage = dstImage->getHandle();
	copyInfo.dstImageLayout = dstImage->getLayout(dstLayer, dstLevel);
	copyInfo.regionCount = 1;
	copyInfo.pRegions = &copiedRegion;
	
	_commandBuffer.copyImage2(copyInfo);
	
	_usedObjects.emplace_back(srcImage);
	_usedObjects.emplace_back(dstImage);
}

void VKCommandBuffer::copyPixelToBuffer(const VKPtr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, glm::uvec2 srcPixel, const VKPtr<VKBufferBase>& dstBuffer, vk::DeviceSize dstByteOffset)
{
	if (srcImage->getPixelByteSize() > dstBuffer->getByteSize() - dstByteOffset)
		throw;
	
	vk::BufferImageCopy2 copiedRegion;
	copiedRegion.bufferOffset = dstByteOffset;
	copiedRegion.bufferRowLength = 0;
	copiedRegion.bufferImageHeight = 0;
	copiedRegion.imageSubresource.aspectMask = VKHelper::getAspect(srcImage->getInfo().getFormat());
	copiedRegion.imageSubresource.mipLevel = srcLevel;
	copiedRegion.imageSubresource.baseArrayLayer = srcLayer;
	copiedRegion.imageSubresource.layerCount = 1;
	copiedRegion.imageOffset = vk::Offset3D(srcPixel.x, srcPixel.y, 0);
	copiedRegion.imageExtent = vk::Extent3D(1, 1, 1);
	
	vk::CopyImageToBufferInfo2 copyInfo;
	copyInfo.srcImage = srcImage->getHandle();
	copyInfo.srcImageLayout = srcImage->getLayout(srcLayer, srcLevel);
	copyInfo.dstBuffer = dstBuffer->getHandle();
	copyInfo.regionCount = 1;
	copyInfo.pRegions = &copiedRegion;
	
	_commandBuffer.copyImageToBuffer2(copyInfo);
	
	_usedObjects.emplace_back(srcImage);
	_usedObjects.emplace_back(dstBuffer);
}

void VKCommandBuffer::blitImage(const VKPtr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, const VKPtr<VKImage>& dstImage, uint32_t dstLayer, uint32_t dstLevel)
{
	vk::ImageBlit2 imageBlit;
	imageBlit.srcSubresource.aspectMask = VKHelper::getAspect(srcImage->getInfo().getFormat());
	imageBlit.srcSubresource.mipLevel = srcLevel;
	imageBlit.srcSubresource.baseArrayLayer = srcLayer;
	imageBlit.srcSubresource.layerCount = 1;
	imageBlit.srcOffsets[0].x = 0;
	imageBlit.srcOffsets[0].y = 0;
	imageBlit.srcOffsets[0].z = 0;
	imageBlit.srcOffsets[1].x = srcImage->getSize(srcLevel).x;
	imageBlit.srcOffsets[1].y = srcImage->getSize(srcLevel).y;
	imageBlit.srcOffsets[1].z = 1;
	imageBlit.dstSubresource.aspectMask = VKHelper::getAspect(dstImage->getInfo().getFormat());
	imageBlit.dstSubresource.mipLevel = dstLevel;
	imageBlit.dstSubresource.baseArrayLayer = dstLayer;
	imageBlit.dstSubresource.layerCount = 1;
	imageBlit.dstOffsets[0].x = 0;
	imageBlit.dstOffsets[0].y = 0;
	imageBlit.dstOffsets[0].z = 0;
	imageBlit.dstOffsets[1].x = dstImage->getSize(dstLevel).x;
	imageBlit.dstOffsets[1].y = dstImage->getSize(dstLevel).y;
	imageBlit.dstOffsets[1].z = 1;
	
	vk::BlitImageInfo2 info;
	info.srcImage = srcImage->getHandle();
	info.srcImageLayout = srcImage->getLayout(srcLayer, srcLevel);
	info.dstImage = dstImage->getHandle();
	info.dstImageLayout = dstImage->getLayout(dstLayer, dstLevel);
	info.regionCount = 1;
	info.pRegions = &imageBlit;
	info.filter = vk::Filter::eLinear;
	
	_commandBuffer.blitImage2(info);
	
	_usedObjects.emplace_back(srcImage);
	_usedObjects.emplace_back(dstImage);
}

void VKCommandBuffer::dispatch(glm::uvec3 groupCount)
{
	if (_boundPipeline == nullptr)
		throw;
	
	if (_boundPipeline->getPipelineType() != vk::PipelineBindPoint::eCompute)
		throw;
	
	_commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
}

void VKCommandBuffer::setViewport(const VKPipelineViewport& viewport)
{
	if (_boundPipeline == nullptr)
		throw;
	
	if (_boundPipeline->getPipelineType() != vk::PipelineBindPoint::eGraphics)
		throw;
	
	vk::Viewport vkViewport;
	vkViewport.x = viewport.offset.x;
	vkViewport.y = viewport.offset.y;
	vkViewport.width = viewport.size.x;
	vkViewport.height = viewport.size.y;
	vkViewport.minDepth = viewport.depthRange.x;
	vkViewport.maxDepth = viewport.depthRange.y;
	
	_commandBuffer.setViewport(0, vkViewport);
}

void VKCommandBuffer::setScissor(const VKPipelineScissor& scissor)
{
	if (_boundPipeline == nullptr)
		throw;
	
	if (_boundPipeline->getPipelineType() != vk::PipelineBindPoint::eGraphics)
		throw;
	
	vk::Rect2D vkScissor;
	vkScissor.offset.x = scissor.offset.x;
	vkScissor.offset.y = scissor.offset.y;
	vkScissor.extent.width = scissor.size.x;
	vkScissor.extent.height = scissor.size.y;
	
	_commandBuffer.setScissor(0, vkScissor);
}

void VKCommandBuffer::pushDebugGroup(std::string_view name)
{
	vk::DebugUtilsLabelEXT label;
	label.pLabelName = name.data();
	label.color[0] = 0.0f;
	label.color[1] = 0.0f;
	label.color[2] = 0.0f;
	label.color[3] = 0.0f;
	
	_commandBuffer.beginDebugUtilsLabelEXT(label);
}

void VKCommandBuffer::popDebugGroup()
{
	_commandBuffer.endDebugUtilsLabelEXT();
}

void VKCommandBuffer::insertTimestamp(const VKPtr<VKTimestampQuery>& timestampQuery)
{
	_commandBuffer.writeTimestamp2(vk::PipelineStageFlagBits2::eAllCommands, timestampQuery->getHandle(), 0);
	
	timestampQuery->setIsInserted(true);
	
	_usedObjects.emplace_back(timestampQuery);
}

void VKCommandBuffer::resetTimestamp(const VKPtr<VKTimestampQuery>& timestampQuery)
{
	_commandBuffer.resetQueryPool(timestampQuery->getHandle(), 0, 1);
	
	timestampQuery->setIsInserted(false);
	
	_usedObjects.emplace_back(timestampQuery);
}

void VKCommandBuffer::queryAccelerationStructureCompactedSize(const VKPtr<VKAccelerationStructure>& accelerationStructure, const VKPtr<VKAccelerationStructureCompactedSizeQuery>& accelerationStructureCompactedSizeQuery)
{
	_commandBuffer.writeAccelerationStructuresPropertiesKHR(
		accelerationStructure->getHandle(),
		vk::QueryType::eAccelerationStructureCompactedSizeKHR,
		accelerationStructureCompactedSizeQuery->getHandle(),
		0);
	
	_usedObjects.emplace_back(accelerationStructure);
	_usedObjects.emplace_back(accelerationStructureCompactedSizeQuery);
}

void VKCommandBuffer::clearColorImage(const VKPtr<VKImage>& image, uint32_t layer, uint32_t level, const vk::ClearColorValue& clearColor)
{
	vk::ImageSubresourceRange range;
	range.aspectMask = VKHelper::getAspect(image->getInfo().getFormat());
	range.baseMipLevel = level;
	range.levelCount = 1;
	range.baseArrayLayer = layer;
	range.layerCount = 1;
	
	_commandBuffer.clearColorImage(image->getHandle(), image->getLayout(layer, level), clearColor, range);
	
	_usedObjects.emplace_back(image);
}

void VKCommandBuffer::buildBottomLevelAccelerationStructure(const VKPtr<VKAccelerationStructure>& accelerationStructure, const VKPtr<VKBufferBase>& scratchBuffer, const VKBottomLevelAccelerationStructureBuildInfo& buildInfo)
{
	if (accelerationStructure->getType() != vk::AccelerationStructureTypeKHR::eBottomLevel)
	{
		throw;
	}
	
	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
	geometry.geometry.triangles = vk::AccelerationStructureGeometryTrianglesDataKHR();
	geometry.geometry.triangles.vertexFormat = buildInfo.vertexFormat;
	geometry.geometry.triangles.vertexData = buildInfo.vertexBuffer->getDeviceAddress();
	geometry.geometry.triangles.vertexStride = buildInfo.vertexStride;
	geometry.geometry.triangles.maxVertex = buildInfo.vertexBuffer->getSize() - 1;
	geometry.geometry.triangles.indexType = buildInfo.indexType;
	geometry.geometry.triangles.indexData = buildInfo.indexBuffer->getDeviceAddress();
	geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
	
	uint32_t primitiveCount = buildInfo.indexBuffer->getSize() / 3;
	
	vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
	buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	buildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction;
	buildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	buildGeometryInfo.dstAccelerationStructure = accelerationStructure->getHandle();
	buildGeometryInfo.geometryCount = 1;
	buildGeometryInfo.pGeometries = &geometry;
	buildGeometryInfo.scratchData = scratchBuffer->getDeviceAddress();
	
	vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo;
	buildRangeInfo.primitiveCount = primitiveCount;
	buildRangeInfo.primitiveOffset = 0;
	buildRangeInfo.firstVertex = 0;
	buildRangeInfo.transformOffset = 0;
	
	_commandBuffer.buildAccelerationStructuresKHR(buildGeometryInfo, &buildRangeInfo);
	
	accelerationStructure->_referencedObjectsInBuild.clear();
	accelerationStructure->_referencedObjectsInBuild.emplace_back(buildInfo.vertexBuffer);
	accelerationStructure->_referencedObjectsInBuild.emplace_back(buildInfo.indexBuffer);
	
	_usedObjects.emplace_back(accelerationStructure);
	_usedObjects.emplace_back(scratchBuffer);
}

void VKCommandBuffer::buildTopLevelAccelerationStructure(const VKPtr<VKAccelerationStructure>& accelerationStructure, const VKPtr<VKBufferBase>& scratchBuffer, const VKTopLevelAccelerationStructureBuildInfo& buildInfo, const VKPtr<VKResizableBuffer<vk::AccelerationStructureInstanceKHR>>& instancesBuffer)
{
	if (accelerationStructure->getType() != vk::AccelerationStructureTypeKHR::eTopLevel)
	{
		throw;
	}
	
	instancesBuffer->resizeSmart(buildInfo.instancesInfos.size());
	
	vk::AccelerationStructureInstanceKHR* instancesBufferPtr = instancesBuffer->getHostPointer();
	for (const VKTopLevelAccelerationStructureBuildInfo::InstanceInfo& instanceInfo : buildInfo.instancesInfos)
	{
		vk::AccelerationStructureInstanceKHR instance;
		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				instance.transform.matrix[x][y] = instanceInfo.localToWorld[y][x];
			}
		}
		instance.instanceCustomIndex = instanceInfo.customIndex;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = {};
		instance.accelerationStructureReference = instanceInfo.accelerationStructure->getDeviceAddress();
		
		std::memcpy(instancesBufferPtr, &instance, sizeof(vk::AccelerationStructureInstanceKHR));
		instancesBufferPtr++;
	}
	
	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eInstances;
	geometry.geometry.instances = vk::AccelerationStructureGeometryInstancesDataKHR();
	geometry.geometry.instances.arrayOfPointers = false;
	geometry.geometry.instances.data = instancesBuffer->getDeviceAddress();
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
	
	uint32_t primitiveCount = buildInfo.instancesInfos.size();
	
	vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
	buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	buildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastBuild;
	buildGeometryInfo.mode = vk::BuildAccelerationStructureModeKHR::eBuild;
	buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	buildGeometryInfo.dstAccelerationStructure = accelerationStructure->getHandle();
	buildGeometryInfo.geometryCount = 1;
	buildGeometryInfo.pGeometries = &geometry;
	buildGeometryInfo.scratchData = scratchBuffer->getDeviceAddress();
	
	vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo;
	buildRangeInfo.primitiveCount = primitiveCount;
	buildRangeInfo.primitiveOffset = 0;
	buildRangeInfo.firstVertex = 0;
	buildRangeInfo.transformOffset = 0;
	
	_commandBuffer.buildAccelerationStructuresKHR(buildGeometryInfo, &buildRangeInfo);
	
	accelerationStructure->_referencedObjectsInBuild.clear();
	for (const VKTopLevelAccelerationStructureBuildInfo::InstanceInfo& instanceInfo : buildInfo.instancesInfos)
	{
		accelerationStructure->_referencedObjectsInBuild.emplace_back(instanceInfo.accelerationStructure);
	}
	accelerationStructure->_referencedObjectsInBuild.emplace_back(instancesBuffer);
	
	_usedObjects.emplace_back(accelerationStructure);
	_usedObjects.emplace_back(scratchBuffer);
}

void VKCommandBuffer::compactAccelerationStructure(const VKPtr<VKAccelerationStructure>& src, const VKPtr<VKAccelerationStructure>& dst)
{
	vk::CopyAccelerationStructureInfoKHR copyAccelerationStructureInfo;
	copyAccelerationStructureInfo.src = src->getHandle();
	copyAccelerationStructureInfo.dst = dst->getHandle();
	copyAccelerationStructureInfo.mode = vk::CopyAccelerationStructureModeKHR::eCompact;
	
	_commandBuffer.copyAccelerationStructureKHR(copyAccelerationStructureInfo);
	
	_usedObjects.emplace_back(src);
	_usedObjects.emplace_back(dst);
}

void VKCommandBuffer::traceRays(const VKPtr<VKBufferBase>& raygenSBT, const VKPtr<VKBufferBase>& missSBT, const VKPtr<VKBufferBase>& hitSBT, glm::uvec2 size)
{
	uint32_t handleSizeAligned = VKHelper::alignUp(32, _context.getRayTracingPipelineProperties().shaderGroupHandleAlignment);
	
	vk::StridedDeviceAddressRegionKHR raygenRegion;
	raygenRegion.deviceAddress = raygenSBT->getDeviceAddress();
	raygenRegion.stride = VKHelper::alignUp(handleSizeAligned, _context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	raygenRegion.size = raygenSBT->getByteSize();
	
	vk::StridedDeviceAddressRegionKHR missRegion;
	missRegion.deviceAddress = missSBT->getDeviceAddress();
	missRegion.stride = VKHelper::alignUp(handleSizeAligned, _context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	missRegion.size = missSBT->getByteSize();
	
	vk::StridedDeviceAddressRegionKHR hitRegion;
	hitRegion.deviceAddress = hitSBT->getDeviceAddress();
	hitRegion.stride = VKHelper::alignUp(handleSizeAligned, _context.getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	hitRegion.size = hitSBT->getByteSize();
	
	vk::StridedDeviceAddressRegionKHR callRegion;
	
	_commandBuffer.traceRaysKHR(
		raygenRegion,
		missRegion,
		hitRegion,
		callRegion,
		size.x,
		size.y,
		1);
	
	_usedObjects.emplace_back(raygenSBT);
	_usedObjects.emplace_back(missSBT);
	_usedObjects.emplace_back(hitSBT);
}

void VKCommandBuffer::pushConstants(const void* data, uint32_t dataSize)
{
	if (_boundPipeline == nullptr)
		throw;
	
	_commandBuffer.pushConstants(_boundPipeline->getPipelineLayout()->getHandle(), vk::ShaderStageFlagBits::eAll, 0, dataSize, data);
}

const VKPtr<VKFence>& VKCommandBuffer::getStatusFence() const
{
	return _statusFence;
}

const VKPtr<VKSemaphore>& VKCommandBuffer::getStatusSemaphore() const
{
	return _statusSemaphore;
}