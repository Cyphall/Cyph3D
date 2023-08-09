#include "PathTracePass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Asset/BindlessTextureManager.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Helper/MathHelper.h"
#include "Cyph3D/VKObject/VKHelper.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKTopLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

PathTracePass::PathTracePass(const glm::uvec2& size):
	RenderPass(size, "Path trace pass")
{
	createBuffers();
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createImage();
}

PathTracePassOutput PathTracePass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, PathTracePassInput& input)
{
	if (input.sceneChanged || input.cameraChanged)
	{
		_accumulatedBatches = 0;
	}
	
	commandBuffer->imageMemoryBarrier(
		_rawRenderImage,
		0,
		0,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral);
	
	bool recreateDescriptorSet = false;
	
	if (input.sceneChanged)
	{
		setupTLAS(commandBuffer, input);
		setupSkyboxUniformsBuffer();
		setupObjectUniformsBuffer(input);
		
		recreateDescriptorSet = true;
	}
	
	if (input.cameraChanged)
	{
		setupCameraUniformsBuffer(input);
		
		recreateDescriptorSet = true;
	}
	
	if (recreateDescriptorSet)
	{
		VKDescriptorSetInfo info(_descriptorSetLayout);
		
		_descriptorSet = VKDescriptorSet::create(Engine::getVKContext(), info);
		
		_descriptorSet->bindAccelerationStructure(0, _tlas);
		_descriptorSet->bindImage(1, _rawRenderImageView);
		_descriptorSet->bindBuffer(2, _cameraUniformsBuffer, 0, 1);
		_descriptorSet->bindBuffer(3, _skyboxUniformsBuffer, 0, 1);
		_descriptorSet->bindBuffer(4, _objectUniformsBuffer, 0, input.registry.getModelRenderRequests().size());
	}
	
	commandBuffer->bindPipeline(_pipeline);
	
	commandBuffer->bindDescriptorSet(0, Engine::getAssetManager().getBindlessTextureManager().getDescriptorSet());
	commandBuffer->bindDescriptorSet(1, _descriptorSet);
	
	FramePushConstants framePushConstants{
		.batchIndex = _batchIndex,
		.sampleCount = input.sampleCount,
		.resetAccumulation = _accumulatedBatches == 0
	};
	
	commandBuffer->pushConstants(framePushConstants);
	
	VKHelper::buildRaygenShaderBindingTable(Engine::getVKContext(), _pipeline, _raygenSBT.getCurrent());
	VKHelper::buildMissShaderBindingTable(Engine::getVKContext(), _pipeline, _missSBT.getCurrent());
	VKHelper::buildHitShaderBindingTable(Engine::getVKContext(), _pipeline, _hitSBT.getCurrent());
	
	commandBuffer->traceRays(_raygenSBT->getBuffer(), _missSBT->getBuffer(), _hitSBT->getBuffer(), _size);
	
	_accumulatedBatches++;
	_batchIndex++;
	
	commandBuffer->unbindPipeline();
	
	return {
		.rawRenderImageView = _rawRenderImageView,
		.accumulatedBatches = _accumulatedBatches
	};
}

void PathTracePass::onResize()
{
	createImage();
}

void PathTracePass::setupTLAS(const VKPtr<VKCommandBuffer>& commandBuffer, const PathTracePassInput& input)
{
	VKTopLevelAccelerationStructureBuildInfo buildInfo;
	buildInfo.instancesInfos.reserve(input.registry.getModelRenderRequests().size());
	for (const ModelRenderer::RenderData& renderData : input.registry.getModelRenderRequests())
	{
		VKTopLevelAccelerationStructureBuildInfo::InstanceInfo& instanceInfo = buildInfo.instancesInfos.emplace_back();
		instanceInfo.localToWorld = renderData.matrix;
		instanceInfo.customIndex = 0;
		instanceInfo.accelerationStructure = renderData.mesh->getAccelerationStructure();
	}
	
	vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = VKAccelerationStructure::getTopLevelBuildSizesInfo(Engine::getVKContext(), buildInfo);
	
	VKBufferInfo backingBufferInfo(buildSizesInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR);
	backingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	VKPtr<VKBuffer<std::byte>> backingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), backingBufferInfo);
	
	_tlas = VKAccelerationStructure::create(
		Engine::getVKContext(),
		vk::AccelerationStructureTypeKHR::eTopLevel,
		buildSizesInfo.accelerationStructureSize,
		backingBuffer);
	
	VKBufferInfo scratchBufferInfo(buildSizesInfo.buildScratchSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
	scratchBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	scratchBufferInfo.setRequiredAlignment(Engine::getVKContext().getAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);
	VKPtr<VKBuffer<std::byte>> scratchBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), scratchBufferInfo);
	
	VKResizableBufferInfo instancesBufferInfo(vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
	instancesBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	instancesBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	instancesBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	instancesBufferInfo.setRequiredAlignment(16);
	VKPtr<VKResizableBuffer<vk::AccelerationStructureInstanceKHR>> instancesBuffer = VKResizableBuffer<vk::AccelerationStructureInstanceKHR>::create(Engine::getVKContext(), instancesBufferInfo);
	
	commandBuffer->buildTopLevelAccelerationStructure(_tlas, scratchBuffer, buildInfo, instancesBuffer);
	
	commandBuffer->bufferMemoryBarrier(
		backingBuffer,
		vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
		vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eAccelerationStructureReadKHR);
}

void PathTracePass::setupCameraUniformsBuffer(const PathTracePassInput& input)
{
	VKBufferInfo cameraUniformsBufferInfo(1, vk::BufferUsageFlagBits::eUniformBuffer);
	cameraUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	cameraUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	cameraUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_cameraUniformsBuffer = VKBuffer<CameraUniforms>::create(Engine::getVKContext(), cameraUniformsBufferInfo);
	
	CameraUniforms cameraUniforms{
		.cameraPosition = input.camera.getPosition(),
		.cameraRayTL = input.camera.getCornerRays()[0],
		.cameraRayTR = input.camera.getCornerRays()[1],
		.cameraRayBL = input.camera.getCornerRays()[2],
		.cameraRayBR = input.camera.getCornerRays()[3]
	};
	
	std::memcpy(_cameraUniformsBuffer->getHostPointer(), &cameraUniforms, sizeof(CameraUniforms));
}

void PathTracePass::setupSkyboxUniformsBuffer()
{
	VKBufferInfo skyboxUniformsBufferInfo(1, vk::BufferUsageFlagBits::eUniformBuffer);
	skyboxUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	skyboxUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	skyboxUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_skyboxUniformsBuffer = VKBuffer<SkyboxUniforms>::create(Engine::getVKContext(), skyboxUniformsBufferInfo);
	
	SkyboxAsset* skybox = Engine::getScene().getSkybox();
	
	SkyboxUniforms skyboxUniforms{};
	if (skybox && skybox->isLoaded())
	{
		skyboxUniforms.hasSkybox = true;
		skyboxUniforms.skyboxIndex = skybox->getBindlessIndex();
		skyboxUniforms.skyboxRotation = glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3(0, 1, 0));
	}
	else
	{
		skyboxUniforms.hasSkybox = false;
	}
	
	std::memcpy(_skyboxUniformsBuffer->getHostPointer(), &skyboxUniforms, sizeof(SkyboxUniforms));
}

void PathTracePass::setupObjectUniformsBuffer(const PathTracePassInput& input)
{
	if (input.registry.getModelRenderRequests().empty())
	{
		_objectUniformsBuffer = {};
	}
	else
	{
		VKBufferInfo objectUniformsBufferInfo(input.registry.getModelRenderRequests().size(), vk::BufferUsageFlagBits::eStorageBuffer);
		objectUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		objectUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
		objectUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
		
		_objectUniformsBuffer = VKBuffer<ObjectUniforms>::create(Engine::getVKContext(), objectUniformsBufferInfo);
		
		ObjectUniforms* objectUniformsPtr = _objectUniformsBuffer->getHostPointer();
		for (const ModelRenderer::RenderData& renderData : input.registry.getModelRenderRequests())
		{
			ObjectUniforms uniforms{
				.normalMatrix = glm::inverseTranspose(glm::mat3(renderData.matrix)),
				.vertexBuffer = renderData.mesh->getFullVertexBuffer()->getDeviceAddress(),
				.indexBuffer = renderData.mesh->getIndexBuffer()->getDeviceAddress(),
				.albedoIndex = renderData.material->getAlbedoTextureBindlessIndex(),
				.normalIndex = renderData.material->getNormalTextureBindlessIndex(),
				.roughnessIndex = renderData.material->getRoughnessTextureBindlessIndex(),
				.metalnessIndex = renderData.material->getMetalnessTextureBindlessIndex(),
				.displacementIndex = renderData.material->getDisplacementTextureBindlessIndex(),
				.emissiveIndex = renderData.material->getEmissiveTextureBindlessIndex(),
				.emissiveScale = renderData.material->getEmissiveScale()
			};
			
			std::memcpy(objectUniformsPtr, &uniforms, sizeof(ObjectUniforms));
			objectUniformsPtr++;
		}
	}
}

void PathTracePass::createBuffers()
{
	VKResizableBufferInfo raygenSBTInfo(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR);
	raygenSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	raygenSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	raygenSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	raygenSBTInfo.setRequiredAlignment(Engine::getVKContext().getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	_raygenSBT = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(context, raygenSBTInfo);
	});
	
	VKResizableBufferInfo missSBTInfo(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR);
	missSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	missSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	missSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	missSBTInfo.setRequiredAlignment(Engine::getVKContext().getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	_missSBT = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(context, missSBTInfo);
	});
	
	VKResizableBufferInfo hitSBTInfo(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR);
	hitSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	hitSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	hitSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	hitSBTInfo.setRequiredAlignment(Engine::getVKContext().getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	_hitSBT = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(context, hitSBTInfo);
	});
}

void PathTracePass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(false);
	info.addBinding(vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eRaygenKHR);
	info.addBinding(vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR);
	info.addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eRaygenKHR);
	info.addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eMissKHR);
	info.addBinding(vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void PathTracePass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(Engine::getAssetManager().getBindlessTextureManager().getDescriptorSetLayout());
	info.addDescriptorSetLayout(_descriptorSetLayout);
	info.setPushConstantLayout<FramePushConstants>(vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void PathTracePass::createPipeline()
{
	VKRayTracingPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/path tracing/path trace.rgen");
	
	info.addRayType("resources/shaders/internal/path tracing/path trace.rmiss");
	info.addObjectTypeForRayType(0, "resources/shaders/internal/path tracing/path trace.rchit", std::nullopt);
	
	_pipeline = VKRayTracingPipeline::create(Engine::getVKContext(), info);
}

void PathTracePass::createImage()
{
	VKImageInfo imageInfo(
		SceneRenderer::ACCUMULATION_FORMAT,
		_size,
		1,
		1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_rawRenderImage = VKImage::create(Engine::getVKContext(), imageInfo);
	
	VKImageViewInfo imageViewInfo(
		_rawRenderImage,
		vk::ImageViewType::e2D);
	
	_rawRenderImageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
	
	_accumulatedBatches = 0;
}