#include "VKRayTracingPipeline.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Shader/VKShader.h"

VKPtr<VKRayTracingPipeline> VKRayTracingPipeline::create(VKContext& context, VKRayTracingPipelineInfo& info)
{
	return VKPtr<VKRayTracingPipeline>(new VKRayTracingPipeline(context, info));
}

VKRayTracingPipeline::VKRayTracingPipeline(VKContext& context, VKRayTracingPipelineInfo& info):
	VKPipeline(context), _info(info)
{
	if (!_context.isRayTracingSupported())
	{
		throw;
	}
	
	std::vector<VKPtr<VKShader>> shaders;
	std::vector<vk::PipelineShaderStageCreateInfo> shadersStagesCreateInfos;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shadersGroupsCreateInfos;
	
	for (const VKRayTracingPipelineInfo::RaygenGroupInfo& groupInfo : _info.getRaygenGroupsInfos())
	{
		VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, groupInfo.raygenShader));
		
		vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo = shadersStagesCreateInfos.emplace_back();
		shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eRaygenKHR;
		shaderStageCreateInfo.module = shader->getHandle();
		shaderStageCreateInfo.pName = "main";
		
		vk::RayTracingShaderGroupCreateInfoKHR& shaderGroupCreateInfo = shadersGroupsCreateInfos.emplace_back();
		shaderGroupCreateInfo.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
		shaderGroupCreateInfo.generalShader = shadersStagesCreateInfos.size() - 1;
		shaderGroupCreateInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroupCreateInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroupCreateInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
	}
	
	for (const VKRayTracingPipelineInfo::TriangleHitGroupInfo& groupInfo : _info.getTriangleHitGroupsInfos())
	{
		vk::RayTracingShaderGroupCreateInfoKHR& shaderGroupCreateInfo = shadersGroupsCreateInfos.emplace_back();
		shaderGroupCreateInfo.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
		shaderGroupCreateInfo.generalShader = VK_SHADER_UNUSED_KHR;
		shaderGroupCreateInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroupCreateInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroupCreateInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
		
		if (groupInfo.closestHitShader)
		{
			VKPtr<VKShader>& closestHitShader = shaders.emplace_back(VKShader::create(_context, *groupInfo.closestHitShader));
			
			vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo = shadersStagesCreateInfos.emplace_back();
			shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eClosestHitKHR;
			shaderStageCreateInfo.module = closestHitShader->getHandle();
			shaderStageCreateInfo.pName = "main";
			
			shaderGroupCreateInfo.closestHitShader = shadersStagesCreateInfos.size() - 1;
		}
		
		if (groupInfo.anyHitShader)
		{
			VKPtr<VKShader>& anyHitShader = shaders.emplace_back(VKShader::create(_context, *groupInfo.anyHitShader));
			
			vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo = shadersStagesCreateInfos.emplace_back();
			shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eAnyHitKHR;
			shaderStageCreateInfo.module = anyHitShader->getHandle();
			shaderStageCreateInfo.pName = "main";
			
			shaderGroupCreateInfo.anyHitShader = shadersStagesCreateInfos.size() - 1;
		}
	}
	
	for (const VKRayTracingPipelineInfo::MissGroupInfo& groupInfo : _info.getMissGroupsInfos())
	{
		VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, groupInfo.missShader));
		
		vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo = shadersStagesCreateInfos.emplace_back();
		shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eMissKHR;
		shaderStageCreateInfo.module = shader->getHandle();
		shaderStageCreateInfo.pName = "main";
		
		vk::RayTracingShaderGroupCreateInfoKHR& shaderGroupCreateInfo = shadersGroupsCreateInfos.emplace_back();
		shaderGroupCreateInfo.type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
		shaderGroupCreateInfo.generalShader = shadersStagesCreateInfos.size() - 1;
		shaderGroupCreateInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroupCreateInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
		shaderGroupCreateInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
	}
	
	vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo;
	pipelineCreateInfo.stageCount = shadersStagesCreateInfos.size();
	pipelineCreateInfo.pStages = shadersStagesCreateInfos.data();
	pipelineCreateInfo.groupCount = shadersGroupsCreateInfos.size();
	pipelineCreateInfo.pGroups = shadersGroupsCreateInfos.data();
	pipelineCreateInfo.maxPipelineRayRecursionDepth = 1;
	pipelineCreateInfo.pLibraryInfo = nullptr;
	pipelineCreateInfo.pLibraryInterface = nullptr;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.layout = _info.getPipelineLayout()->getHandle();
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	_pipeline = _context.getDevice().createRayTracingPipelineKHR({}, VK_NULL_HANDLE, pipelineCreateInfo).value;
	
	_raygenGroupsHandles.resize(_info.getRaygenGroupsInfos().size());
	_context.getDevice().getRayTracingShaderGroupHandlesKHR(
		_pipeline,
		0,
		_info.getRaygenGroupsInfos().size(),
		_raygenGroupsHandles.size() * 32, _raygenGroupsHandles.data());
	
	_triangleHitGroupsHandles.resize(_info.getTriangleHitGroupsInfos().size());
	_context.getDevice().getRayTracingShaderGroupHandlesKHR(
		_pipeline,
		_info.getRaygenGroupsInfos().size(),
		_info.getTriangleHitGroupsInfos().size(),
		_triangleHitGroupsHandles.size() * 32, _triangleHitGroupsHandles.data());
	
	_missGroupsHandles.resize(_info.getMissGroupsInfos().size());
	_context.getDevice().getRayTracingShaderGroupHandlesKHR(
		_pipeline,
		_info.getRaygenGroupsInfos().size() + _info.getTriangleHitGroupsInfos().size(),
		_info.getMissGroupsInfos().size(),
		_missGroupsHandles.size() * 32, _missGroupsHandles.data());
}

VKRayTracingPipeline::~VKRayTracingPipeline()
{
	_context.getDevice().destroyPipeline(_pipeline);
}

const VKRayTracingPipelineInfo& VKRayTracingPipeline::getInfo() const
{
	return _info;
}

const std::array<std::byte, 32>& VKRayTracingPipeline::getRaygenGroupHandle(uint32_t index) const
{
	return _raygenGroupsHandles[index];
}

size_t VKRayTracingPipeline::getRaygenGroupCount() const
{
	return _raygenGroupsHandles.size();
}

const std::array<std::byte, 32>& VKRayTracingPipeline::getTriangleHitGroupHandle(uint32_t index) const
{
	return _triangleHitGroupsHandles[index];
}

size_t VKRayTracingPipeline::getTriangleHitGroupCount() const
{
	return _triangleHitGroupsHandles.size();
}

const std::array<std::byte, 32>& VKRayTracingPipeline::getMissGroupHandle(uint32_t index) const
{
	return _missGroupsHandles[index];
}

size_t VKRayTracingPipeline::getMissGroupCount() const
{
	return _missGroupsHandles.size();
}

vk::PipelineBindPoint VKRayTracingPipeline::getPipelineType() const
{
	return vk::PipelineBindPoint::eRayTracingKHR;
}

const VKPtr<VKPipelineLayout>& VKRayTracingPipeline::getPipelineLayout() const
{
	return _info.getPipelineLayout();
}
