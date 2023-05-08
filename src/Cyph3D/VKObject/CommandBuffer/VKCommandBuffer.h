#pragma once

#include "Cyph3D/VKObject/VKObject.h"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

class VKBufferBase;
template<typename T>
class VKResizableBuffer;
class VKImage;
class VKImageView;
class VKSampler;
class VKDescriptorSet;
class VKPipeline;
class VKFence;
class VKSemaphore;
class VKTimestampQuery;
class VKAccelerationStructure;
struct VKPipelineViewport;
struct VKPipelineScissor;
struct VKBottomLevelAccelerationStructureBuildInfo;
struct VKTopLevelAccelerationStructureBuildInfo;

class VKCommandBuffer : public VKObject
{
public:
	static VKPtr<VKCommandBuffer> create(VKContext& context);
	static VKDynamic<VKCommandBuffer> createDynamic(VKContext& context);
	
	~VKCommandBuffer() override;
	
	const vk::CommandBuffer& getHandle();
	
	bool waitExecution() const;
	
	void begin();
	void end();
	void reset();
	
	void bufferMemoryBarrier(const VKPtr<VKBufferBase>& buffer, vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask);
	void imageMemoryBarrier(const VKPtr<VKImage>& image, vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask);
	void imageMemoryBarrier(const VKPtr<VKImage>& image, vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask, vk::ImageLayout newImageLayout, uint32_t layer, uint32_t level);
	
	void beginRendering(const vk::RenderingInfo& renderingInfo);
	void endRendering();
	
	void bindPipeline(const VKPtr<VKPipeline>& pipeline);
	void unbindPipeline();
	
	void bindDescriptorSet(uint32_t setIndex, const VKPtr<VKDescriptorSet>& descriptorSet);
	void bindDescriptorSet(uint32_t setIndex, const VKPtr<VKDescriptorSet>& descriptorSet, uint32_t dynamicOffset);
	
	template<typename T>
	void pushConstants(const T& data)
	{
		pushConstants(&data, sizeof(T));
	}
	
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKBufferBase>& buffer, size_t offset, size_t size, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKImageView>& imageView, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKImageView>& imageView, const VKPtr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const VKPtr<VKAccelerationStructure>& accelerationStructure, uint32_t arrayIndex = 0);
	
	void bindVertexBuffer(uint32_t vertexBufferIndex, const VKPtr<VKBufferBase>& vertexBuffer);
	void bindIndexBuffer(const VKPtr<VKBufferBase>& indexBuffer);
	
	void draw(uint32_t vertexCount, uint32_t vertexOffset);
	void drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset);
	
	void copyBufferToImage(const VKPtr<VKBufferBase>& srcBuffer, vk::DeviceSize srcByteOffset, const VKPtr<VKImage>& dstImage, uint32_t dstLayer, uint32_t dstLevel);
	
	void copyBufferToBuffer(const VKPtr<VKBufferBase>& srcBuffer, vk::DeviceSize srcByteOffset, const VKPtr<VKBufferBase>& dstBuffer, vk::DeviceSize dstByteOffset, vk::DeviceSize size);
	
	void copyImageToBuffer(const VKPtr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, const VKPtr<VKBufferBase>& dstBuffer, vk::DeviceSize dstByteOffset);
	
	void copyImageToImage(const VKPtr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, const VKPtr<VKImage>& dstImage, uint32_t dstLayer, uint32_t dstLevel);
	
	void copyPixelToBuffer(const VKPtr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, glm::uvec2 srcPixel, const VKPtr<VKBufferBase>& dstBuffer, vk::DeviceSize dstByteOffset);
	
	void blitImage(const VKPtr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, const VKPtr<VKImage>& dstImage, uint32_t dstLayer, uint32_t dstLevel);
	
	void dispatch(glm::uvec3 groupCount);
	
	void setViewport(const VKPipelineViewport& viewport);
	
	void setScissor(const VKPipelineScissor& scissor);
	
	void pushDebugGroup(std::string_view name);
	void popDebugGroup();
	
	void insertTimestamp(const VKPtr<VKTimestampQuery>& timestampQuery);
	void resetTimestamp(const VKPtr<VKTimestampQuery>& timestampQuery);
	
	void clearColorImage(const VKPtr<VKImage>& image, uint32_t layer, uint32_t level, const vk::ClearColorValue& clearColor);
	
	void buildBottomLevelAccelerationStructure(const VKPtr<VKAccelerationStructure>& accelerationStructure, const VKPtr<VKBufferBase>& scratchBuffer, const VKBottomLevelAccelerationStructureBuildInfo& buildInfo);
	void buildTopLevelAccelerationStructure(const VKPtr<VKAccelerationStructure>& accelerationStructure, const VKPtr<VKBufferBase>& scratchBuffer, const VKTopLevelAccelerationStructureBuildInfo& buildInfo, const VKPtr<VKResizableBuffer<vk::AccelerationStructureInstanceKHR>>& instancesBuffer);
	
	void traceRays(const VKPtr<VKBufferBase>& raygenSBT, const VKPtr<VKBufferBase>& missSBT, const VKPtr<VKBufferBase>& hitSBT, glm::uvec2 size);
	
	const VKPtr<VKFence>& getStatusFence() const;
	const VKPtr<VKSemaphore>& getStatusSemaphore() const;
	
private:
	explicit VKCommandBuffer(VKContext& context);
	
	void pushConstants(const void* data, uint32_t dataSize);
	
	vk::CommandPool _commandPool;
	vk::CommandBuffer _commandBuffer;
	
	VKPtr<VKFence> _statusFence;
	VKPtr<VKSemaphore> _statusSemaphore;
	
	VKPipeline* _boundPipeline = nullptr;
	
	std::vector<VKPtr<VKObject>> _usedObjects;
};