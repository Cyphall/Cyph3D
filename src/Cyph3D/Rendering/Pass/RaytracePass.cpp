#include "RaytracePass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Helper/MathHelper.h"
#include "Cyph3D/VKObject/VKHelper.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKTopLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.h"

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
		_rawRenderImage.getVKPtr(),
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral,
		0,
		0);
	
	commandBuffer->imageMemoryBarrier(
		_objectIndexImage.getVKPtr(),
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral,
		0,
		0);
	
	VKTopLevelAccelerationStructureBuildInfo buildInfo;
	buildInfo.instancesInfos.reserve(input.registry.models.size());
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
	}
	
	vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = VKAccelerationStructure::getTopLevelBuildSizesInfo(Engine::getVKContext(), buildInfo);
	
	_tlasBackingBuffer->resizeSmart(buildSizesInfo.accelerationStructureSize);
	
	VKPtr<VKAccelerationStructure> tlas = VKAccelerationStructure::create(
		Engine::getVKContext(),
		vk::AccelerationStructureTypeKHR::eTopLevel,
		buildSizesInfo.accelerationStructureSize,
		_tlasBackingBuffer->getBuffer());
	
	_tlasScratchBuffer->resizeSmart(buildSizesInfo.buildScratchSize);
	
	commandBuffer->buildTopLevelAccelerationStructure(tlas, _tlasScratchBuffer->getBuffer(), buildInfo, _tlasInstancesBuffer.getVKPtr());
	
	commandBuffer->bufferMemoryBarrier(
		_tlasBackingBuffer->getBuffer(),
		vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
		vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eAccelerationStructureReadKHR);
	
	commandBuffer->bindPipeline(_pipeline);
	
	commandBuffer->pushDescriptor(0, 0, tlas);
	commandBuffer->pushDescriptor(0, 1, _rawRenderImageView.getVKPtr());
	commandBuffer->pushDescriptor(0, 2, _objectIndexImageView.getVKPtr());
	
	PushConstantData pushConstantData;
	pushConstantData.position = input.camera.getPosition();
	pushConstantData.rayTL = input.camera.getCornerRays()[0];
	pushConstantData.rayTR = input.camera.getCornerRays()[1];
	pushConstantData.rayBL = input.camera.getCornerRays()[2];
	pushConstantData.rayBR = input.camera.getCornerRays()[3];
	commandBuffer->pushConstants(pushConstantData);
	
	VKHelper::buildRaygenShaderBindingTable(Engine::getVKContext(), _pipeline, _raygenSBT.getVKPtr());
	VKHelper::buildMissShaderBindingTable(Engine::getVKContext(), _pipeline, _missSBT.getVKPtr());
	VKHelper::buildHitShaderBindingTable(Engine::getVKContext(), _pipeline, _hitSBT.getVKPtr());
	
	commandBuffer->traceRays(_raygenSBT->getBuffer(), _missSBT->getBuffer(), _hitSBT->getBuffer(), _size);
	
	commandBuffer->unbindPipeline();
	
	return {
		.rawRenderImageView = _rawRenderImageView.getVKPtr(),
		.objectIndexImageView = _objectIndexImageView.getVKPtr()
	};
}

void RaytracePass::onResize()
{
	createImages();
}

void RaytracePass::createBuffers()
{
	_tlasBackingBuffer = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_tlasScratchBuffer = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_tlasInstancesBuffer = VKResizableBuffer<vk::AccelerationStructureInstanceKHR>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_raygenSBT = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_missSBT = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_hitSBT = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

void RaytracePass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eAccelerationStructureKHR, 1);
	info.addBinding(vk::DescriptorType::eStorageImage, 1);
	info.addBinding(vk::DescriptorType::eStorageImage, 1);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void RaytracePass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(_descriptorSetLayout);
	info.setPushConstantLayout<PushConstantData>();
	
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
		_rawRenderImage = VKImage::createDynamic(
			Engine::getVKContext(),
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_rawRenderImageView = VKImageView::createDynamic(
			Engine::getVKContext(),
			_rawRenderImage,
			vk::ImageViewType::e2D);
	}
	
	{
		_objectIndexImage = VKImage::createDynamic(
			Engine::getVKContext(),
			SceneRenderer::OBJECT_INDEX_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_objectIndexImageView = VKImageView::createDynamic(
			Engine::getVKContext(),
			_objectIndexImage,
			vk::ImageViewType::e2D);
	}
}