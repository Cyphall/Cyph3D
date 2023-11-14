#include "ShadowMapPass.h"

#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/Helper/MathHelper.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Scene/Transform.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"

#include <glm/gtc/matrix_inverse.hpp>

static const float POINT_SHADOW_MAP_NEAR = 0.01f;
static const float POINT_SHADOW_MAP_FAR = 100.0f;

static const glm::mat4 POINT_SHADOW_MAP_PROJECTION = []()
{
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, POINT_SHADOW_MAP_NEAR, POINT_SHADOW_MAP_FAR);
	
	projection[1][1] *= -1;
	
	return projection;
}();

static glm::mat4 calcDirectionalShadowMapView(const DirectionalLight::RenderData& light)
{
	return glm::lookAt(
		{0, 0, 0},
		light.transform.getDown(),
		light.transform.getForward());
}

static std::tuple<glm::mat4, float, float> calcDirectionalShadowMapProjection(const glm::mat4& view, const std::vector<ModelRenderer::RenderData>& models)
{
	glm::vec3 min(std::numeric_limits<float>::max());
	glm::vec3 max(std::numeric_limits<float>::lowest());
	
	for (const ModelRenderer::RenderData& model : models)
	{
		glm::mat4 modelView = view * model.transform.getLocalToWorldMatrix();
		
		auto [boundingBoxMin_SMS, boundingBoxMax_SMS] = MathHelper::transformBoundingBox(
			modelView,
			model.mesh.getBoundingBoxMin(),
			model.mesh.getBoundingBoxMax());
		
		min = glm::min(min, boundingBoxMin_SMS);
		max = glm::max(max, boundingBoxMax_SMS);
	}
	
	min -= 0.01f;
	max += 0.01f;
	
	glm::vec3 center = (max + min) / 2.0f;
	
	glm::vec3 size = max - min;
	size.x = size.y = std::max(size.x, size.y); // force the matrix to have the same width and height to get square shadow map pixels in worldspace
	
	glm::mat4 projection = glm::ortho(
		center.x - size.x / 2, center.x + size.x / 2,
		center.y + size.y / 2, center.y - size.y / 2,
		-center.z - size.z / 2, -center.z + size.z / 2);
	
	return {projection, size.x, size.z};
}

static std::array<glm::mat4, 6> calcPointShadowMapView(const PointLight::RenderData& light)
{
	glm::vec3 position = light.transform.getWorldPosition();
	
	return {
		glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)),
		glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0)),
		glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
		glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
		glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)),
		glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0))
	};
}

ShadowMapPass::ShadowMapPass(glm::uvec2 size):
	RenderPass(size, "Shadow map pass")
{
	createDescriptorSetLayout();
	createBuffer();
	createPipelineLayouts();
	createPipelines();
}

ShadowMapPassOutput ShadowMapPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ShadowMapPassInput& input)
{
	if (input.sceneChanged || input.cameraChanged)
	{
		_shadowMapManager.resetDirectionalShadowMapAllocations();
		_directionalShadowMapInfos.clear();
		
		int shadowCastingDirectionalLightIndex = 0;
		for (const DirectionalLight::RenderData& light : input.registry.getDirectionalLightRenderRequests())
		{
			if (!light.castShadows)
			{
				continue;
			}
			
			commandBuffer->pushDebugGroup(std::format("Directional light ({})", shadowCastingDirectionalLightIndex));
			renderDirectionalShadowMap(commandBuffer, light, input.registry.getModelRenderRequests());
			commandBuffer->popDebugGroup();
			
			shadowCastingDirectionalLightIndex++;
		}
	}
	
	if (input.sceneChanged)
	{
		_shadowMapManager.resetPointShadowMapAllocations();
		_pointShadowMapInfos.clear();
		
		int shadowCastingPointLights = 0;
		for (const PointLight::RenderData& light : input.registry.getPointLightRenderRequests())
		{
			if (light.castShadows)
			{
				shadowCastingPointLights++;
			}
		}
		
		_pointLightUniformBuffer->resizeSmart(shadowCastingPointLights * 6);
		
		int shadowCastingPointLightIndex = 0;
		for (const PointLight::RenderData& light : input.registry.getPointLightRenderRequests())
		{
			if (!light.castShadows)
			{
				continue;
			}
			
			commandBuffer->pushDebugGroup(std::format("Point light ({})", shadowCastingPointLightIndex));
			renderPointShadowMap(commandBuffer, light, input.registry.getModelRenderRequests(), shadowCastingPointLightIndex);
			commandBuffer->popDebugGroup();
			
			shadowCastingPointLightIndex++;
		}
	}
	
	return {
		.directionalShadowMapInfos = _directionalShadowMapInfos,
		.pointShadowMapInfos = _pointShadowMapInfos
	};
}

void ShadowMapPass::onResize()
{

}

void ShadowMapPass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eStorageBuffer, 1);
	
	_pointLightDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void ShadowMapPass::createBuffer()
{
	VKResizableBufferInfo bufferInfo(vk::BufferUsageFlagBits::eStorageBuffer);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	bufferInfo.setName("Point light shadow generation uniform buffer");
	
	_pointLightUniformBuffer = VKDynamic<VKResizableBuffer<PointLightUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<PointLightUniforms>::create(context, bufferInfo);
	});
}

void ShadowMapPass::createPipelineLayouts()
{
	{
		VKPipelineLayoutInfo info;
		info.setPushConstantLayout<DirectionalLightPushConstantData>();
		
		_directionalLightPipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKPipelineLayoutInfo info;
		info.addDescriptorSetLayout(_pointLightDescriptorSetLayout);
		info.setPushConstantLayout<PointLightPushConstantData>();
		
		_pointLightPipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
}

void ShadowMapPass::createPipelines()
{
	{
		VKGraphicsPipelineInfo info(
			_directionalLightPipelineLayout,
			"resources/shaders/internal/shadow mapping/directional light.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		
		info.getVertexInputLayoutInfo().defineSlot(0, sizeof(PositionVertexData), vk::VertexInputRate::eVertex);
		info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(PositionVertexData, position));
		
		info.getPipelineAttachmentInfo().setDepthAttachment(SceneRenderer::DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT, vk::CompareOp::eLess, true);
		
		_directionalLightPipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
	
	{
		VKGraphicsPipelineInfo info(
			_pointLightPipelineLayout,
			"resources/shaders/internal/shadow mapping/point light.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		
		info.setFragmentShader("resources/shaders/internal/shadow mapping/point light.frag");
		
		info.getVertexInputLayoutInfo().defineSlot(0, sizeof(PositionVertexData), vk::VertexInputRate::eVertex);
		info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(PositionVertexData, position));
		
		info.getPipelineAttachmentInfo().setDepthAttachment(SceneRenderer::POINT_SHADOW_MAP_DEPTH_FORMAT, vk::CompareOp::eLess, true);
		
		_pointLightPipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
}

void ShadowMapPass::renderDirectionalShadowMap(
	const VKPtr<VKCommandBuffer>& commandBuffer,
	const DirectionalLight::RenderData& light,
	const std::vector<ModelRenderer::RenderData>& models)
{
	VKPtr<VKImage> shadowMap = _shadowMapManager.allocateDirectionalShadowMap(light.shadowMapResolution);
	
	commandBuffer->imageMemoryBarrier(
		shadowMap,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eDepthAttachmentOptimal);
	
	VKRenderingInfo renderingInfo({light.shadowMapResolution, light.shadowMapResolution});
	
	renderingInfo.setDepthAttachment(shadowMap)
		.setLoadOpClear(1.0f)
		.setStoreOpStore();
	
	commandBuffer->beginRendering(renderingInfo);
	
	commandBuffer->bindPipeline(_directionalLightPipeline);
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = {light.shadowMapResolution, light.shadowMapResolution};
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = {light.shadowMapResolution, light.shadowMapResolution};
	commandBuffer->setScissor(scissor);
	
	glm::mat4 view = calcDirectionalShadowMapView(light);
	auto [projection, worldSize, worldDepth] = calcDirectionalShadowMapProjection(view, models);
	glm::mat4 viewProjection = projection * view;
	
	for (const ModelRenderer::RenderData& model : models)
	{
		if (!model.contributeShadows)
		{
			continue;
		}
		
		const VKPtr<VKBuffer<PositionVertexData>>& vertexBuffer = model.mesh.getPositionVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = model.mesh.getIndexBuffer();
		
		commandBuffer->bindVertexBuffer(0, vertexBuffer);
		commandBuffer->bindIndexBuffer(indexBuffer);
		
		DirectionalLightPushConstantData pushConstantData{};
		pushConstantData.mvp = viewProjection * model.transform.getLocalToWorldMatrix();
		commandBuffer->pushConstants(pushConstantData);
		
		commandBuffer->drawIndexed(indexBuffer->getSize(), 0, 0);
	}
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	_directionalShadowMapInfos.push_back(DirectionalShadowMapInfo{
		.worldSize = worldSize,
		.worldDepth = worldDepth,
		.viewProjection = viewProjection,
		.image = shadowMap
	});
}

void ShadowMapPass::renderPointShadowMap(
	const VKPtr<VKCommandBuffer>& commandBuffer,
	const PointLight::RenderData& light,
	const std::vector<ModelRenderer::RenderData>& models,
	int uniformIndex)
{
	VKPtr<VKImage> shadowMap = _shadowMapManager.allocatePointShadowMap(light.shadowMapResolution);
	
	commandBuffer->imageMemoryBarrier(
		shadowMap,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentRead,
		vk::ImageLayout::eDepthAttachmentOptimal);
	
	std::array<glm::mat4, 6> views = calcPointShadowMapView(light);
	
	for (int i = 0; i < 6; i++)
	{
		VKRenderingInfo renderingInfo({light.shadowMapResolution, light.shadowMapResolution});
		
		renderingInfo.setDepthAttachment(
			shadowMap,
			vk::ImageViewType::e2D,
			{i, i},
			{0, 0},
			shadowMap->getInfo().getFormat())
			.setLoadOpClear(std::numeric_limits<float>::max())
			.setStoreOpStore();
		
		commandBuffer->beginRendering(renderingInfo);
		
		commandBuffer->bindPipeline(_pointLightPipeline);
		
		VKPipelineViewport viewport;
		viewport.offset = {0, 0};
		viewport.size = {light.shadowMapResolution, light.shadowMapResolution};
		viewport.depthRange = {0.0f, 1.0f};
		commandBuffer->setViewport(viewport);
		
		VKPipelineScissor scissor;
		scissor.offset = {0, 0};
		scissor.size = {light.shadowMapResolution, light.shadowMapResolution};
		commandBuffer->setScissor(scissor);
		
		int uniformOffset = uniformIndex * 6 + i;
		
		PointLightUniforms uniforms{
			.viewProjection = POINT_SHADOW_MAP_PROJECTION * views[i],
			.lightPos = light.transform.getWorldPosition()
		};
		std::memcpy(_pointLightUniformBuffer->getHostPointer() + uniformOffset, &uniforms, sizeof(PointLightUniforms));
		
		commandBuffer->pushDescriptor(0, 0, _pointLightUniformBuffer.getCurrent()->getBuffer(), uniformOffset, 1);
		
		for (const ModelRenderer::RenderData& model : models)
		{
			if (!model.contributeShadows)
			{
				continue;
			}
			
			const VKPtr<VKBuffer<PositionVertexData>>& vertexBuffer = model.mesh.getPositionVertexBuffer();
			const VKPtr<VKBuffer<uint32_t>>& indexBuffer = model.mesh.getIndexBuffer();
			
			commandBuffer->bindVertexBuffer(0, vertexBuffer);
			commandBuffer->bindIndexBuffer(indexBuffer);
			
			PointLightPushConstantData pushConstantData{};
			pushConstantData.model = model.transform.getLocalToWorldMatrix();
			commandBuffer->pushConstants(pushConstantData);
			
			commandBuffer->drawIndexed(indexBuffer->getSize(), 0, 0);
		}
		
		commandBuffer->unbindPipeline();
		
		commandBuffer->endRendering();
	}
	
	_pointShadowMapInfos.push_back(PointShadowMapInfo{
		.image = shadowMap
	});
}
