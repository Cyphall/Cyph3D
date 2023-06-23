#include "RaytracePass.h"

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
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

RaytracePass::RaytracePass(const glm::uvec2& size):
	RenderPass(size, "Raytrace pass")
{
	createBuffers();
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createImages();
}

RaytracePassOutput RaytracePass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, RaytracePassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		_rawRenderImage.getCurrent(),
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral);
	
	commandBuffer->imageMemoryBarrier(
		_objectIndexImage.getCurrent(),
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral);
	
	VKTopLevelAccelerationStructureBuildInfo buildInfo;
	buildInfo.instancesInfos.reserve(input.registry.models.size());
	_objectUniforms->resizeSmart(input.registry.models.size());
	ObjectUniforms* objectUniformsPtr = _objectUniforms->getHostPointer();
	int actualObjectCountToBeDrawn = 0;
	for (int i = 0; i < input.registry.models.size(); i++)
	{
		ModelRenderer::RenderData modelRenderData = input.registry.models[i];
		
		MaterialAsset* material = modelRenderData.material;
		if (material == nullptr)
		{
			material = MaterialAsset::getMissingMaterial();
		}
		else if (!material->isLoaded())
		{
			material = MaterialAsset::getDefaultMaterial();
		}
		
		if (material == nullptr || !material->isLoaded())
		{
			continue;
		}
		
		MeshAsset* mesh = modelRenderData.mesh;
		if (mesh == nullptr || !mesh->isLoaded())
		{
			continue;
		}
		
		VKTopLevelAccelerationStructureBuildInfo::InstanceInfo& instanceInfo = buildInfo.instancesInfos.emplace_back();
		instanceInfo.localToWorld = modelRenderData.matrix;
		instanceInfo.customIndex = i;
		instanceInfo.accelerationStructure = mesh->getAccelerationStructure();
		
		ObjectUniforms uniforms{};
		uniforms.normalMatrix = glm::inverseTranspose(glm::mat3(modelRenderData.matrix));
		uniforms.model = modelRenderData.matrix;
		uniforms.vertexBuffer = mesh->getVertexBuffer()->getDeviceAddress();
		uniforms.indexBuffer = mesh->getIndexBuffer()->getDeviceAddress();
		uniforms.albedoIndex = material->getAlbedoTextureBindlessIndex();
		uniforms.normalIndex = material->getNormalTextureBindlessIndex();
		uniforms.roughnessIndex = material->getRoughnessTextureBindlessIndex();
		uniforms.metalnessIndex = material->getMetalnessTextureBindlessIndex();
		uniforms.displacementIndex = material->getDisplacementTextureBindlessIndex();
		uniforms.emissiveIndex = material->getEmissiveTextureBindlessIndex();
		uniforms.emissiveScale = material->getEmissiveScale();
		std::memcpy(objectUniformsPtr, &uniforms, sizeof(ObjectUniforms));
		objectUniformsPtr++;
		
		actualObjectCountToBeDrawn++;
	}
	
	vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = VKAccelerationStructure::getTopLevelBuildSizesInfo(Engine::getVKContext(), buildInfo);
	
	_tlasBackingBuffer->resizeSmart(buildSizesInfo.accelerationStructureSize);
	
	VKPtr<VKAccelerationStructure> tlas = VKAccelerationStructure::create(
		Engine::getVKContext(),
		vk::AccelerationStructureTypeKHR::eTopLevel,
		buildSizesInfo.accelerationStructureSize,
		_tlasBackingBuffer->getBuffer());
	
	_tlasScratchBuffer->resizeSmart(buildSizesInfo.buildScratchSize);
	
	commandBuffer->buildTopLevelAccelerationStructure(tlas, _tlasScratchBuffer->getBuffer(), buildInfo, _tlasInstancesBuffer.getCurrent());
	
	commandBuffer->bufferMemoryBarrier(
		_tlasBackingBuffer->getBuffer(),
		vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
		vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eAccelerationStructureReadKHR);
	
	GlobalUniforms globalUniforms;
	globalUniforms.cameraPosition = input.camera.getPosition();
	globalUniforms.cameraRayTL = input.camera.getCornerRays()[0];
	globalUniforms.cameraRayTR = input.camera.getCornerRays()[1];
	globalUniforms.cameraRayBL = input.camera.getCornerRays()[2];
	globalUniforms.cameraRayBR = input.camera.getCornerRays()[3];
	globalUniforms.frameIndex = _frameIndex;

	SkyboxAsset* skybox = Engine::getScene().getSkybox();
	if (skybox && skybox->isLoaded())
	{
		globalUniforms.hasSkybox = true;
		globalUniforms.skyboxIndex = skybox->getBindlessIndex();
		globalUniforms.skyboxRotation = glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3(0, 1, 0));
	}
	else
	{
		globalUniforms.hasSkybox = false;
	}
	
	std::memcpy(_globalUniforms->getHostPointer(), &globalUniforms, sizeof(GlobalUniforms));
	
	commandBuffer->bindPipeline(_pipeline);
	
	commandBuffer->bindDescriptorSet(0, Engine::getAssetManager().getBindlessTextureManager().getDescriptorSet());
	commandBuffer->pushDescriptor(1, 0, tlas);
	commandBuffer->pushDescriptor(1, 1, _rawRenderImageView.getCurrent());
	commandBuffer->pushDescriptor(1, 2, _objectIndexImageView.getCurrent());
	commandBuffer->pushDescriptor(1, 3, _globalUniforms.getCurrent(), 0, 1);
	commandBuffer->pushDescriptor(1, 4, _objectUniforms->getBuffer(), 0, actualObjectCountToBeDrawn);
	
	VKHelper::buildRaygenShaderBindingTable(Engine::getVKContext(), _pipeline, _raygenSBT.getCurrent());
	VKHelper::buildMissShaderBindingTable(Engine::getVKContext(), _pipeline, _missSBT.getCurrent());
	VKHelper::buildHitShaderBindingTable(Engine::getVKContext(), _pipeline, _hitSBT.getCurrent());
	
	commandBuffer->traceRays(_raygenSBT->getBuffer(), _missSBT->getBuffer(), _hitSBT->getBuffer(), _size);
	
	commandBuffer->unbindPipeline();
	
	_frameIndex++;
	
	return {
		.rawRenderImageView = _rawRenderImageView.getCurrent(),
		.objectIndexImageView = _objectIndexImageView.getCurrent()
	};
}

void RaytracePass::onResize()
{
	createImages();
}

void RaytracePass::createBuffers()
{
	_globalUniforms = VKDynamic<VKBuffer<GlobalUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKBuffer<GlobalUniforms>::create(
			context,
			1,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	});
	
	_objectUniforms = VKDynamic<VKResizableBuffer<ObjectUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<ObjectUniforms>::create(
			context,
			vk::BufferUsageFlagBits::eStorageBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	});
	
	_tlasBackingBuffer = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(
			context,
			vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	});
	
	_tlasScratchBuffer = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(
			context,
			vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	});
	
	_tlasInstancesBuffer = VKDynamic<VKResizableBuffer<vk::AccelerationStructureInstanceKHR>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<vk::AccelerationStructureInstanceKHR>::create(
			context,
			vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	});
	
	_raygenSBT = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(
			context,
			vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	});
	
	_missSBT = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(
			context,
			vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	});
	
	_hitSBT = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(
			context,
			vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
			vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	});
}

void RaytracePass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eAccelerationStructureKHR, 1);
	info.addBinding(vk::DescriptorType::eStorageImage, 1);
	info.addBinding(vk::DescriptorType::eStorageImage, 1);
	info.addBinding(vk::DescriptorType::eUniformBuffer, 1);
	info.addBinding(vk::DescriptorType::eStorageBuffer, 1);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void RaytracePass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(Engine::getAssetManager().getBindlessTextureManager().getDescriptorSetLayout());
	info.addDescriptorSetLayout(_descriptorSetLayout);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void RaytracePass::createPipeline()
{
	VKRayTracingPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/raytracing/raytrace.rgen");
	
	info.addRayType("resources/shaders/internal/raytracing/raytrace.rmiss");
	info.addObjectTypeForRayType(0, "resources/shaders/internal/raytracing/raytrace.rchit", std::nullopt, std::nullopt);
	
	_pipeline = VKRayTracingPipeline::create(Engine::getVKContext(), info);
}

void RaytracePass::createImages()
{
	{
		VKImageInfo imageInfo(
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_rawRenderImage = VKDynamic<VKImage>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKImage::create(context, imageInfo);
		});
		
		_rawRenderImageView = VKDynamic<VKImageView>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			VKImageViewInfo imageViewInfo(
				_rawRenderImage[index],
				vk::ImageViewType::e2D);
			
			return VKImageView::create(context, imageViewInfo);
		});
	}
	
	{
		VKImageInfo imageInfo(
			SceneRenderer::OBJECT_INDEX_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc);
		imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_objectIndexImage = VKDynamic<VKImage>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			return VKImage::create(context, imageInfo);
		});
		
		_objectIndexImageView = VKDynamic<VKImageView>(Engine::getVKContext(), [&](VKContext& context, int index)
		{
			VKImageViewInfo imageViewInfo(
				_objectIndexImage[index],
				vk::ImageViewType::e2D);
			
			return VKImageView::create(context, imageViewInfo);
		});
	}
}