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
	
	{
		VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, _info.getRaygenShader()));
		
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
	
	for (const VKRayTracingPipelineInfo::RayTypeInfo& rayTypeInfo : _info.getRayTypesInfos())
	{
		{
			VKPtr<VKShader>& shader = shaders.emplace_back(VKShader::create(_context, rayTypeInfo.missShader));
			
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
		
		for (const VKRayTracingPipelineInfo::ObjectTypeInfo& objectTypeInfo : rayTypeInfo.objectTypesInfos)
		{
			int64_t closestHitIndex = -1;
			int64_t anyHitIndex = -1;
			int64_t intersectionIndex = -1;
			
			if (objectTypeInfo.closestHitShader)
			{
				VKPtr<VKShader>& closestHitShader = shaders.emplace_back(VKShader::create(_context, *objectTypeInfo.closestHitShader));
				
				vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo = shadersStagesCreateInfos.emplace_back();
				shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eClosestHitKHR;
				shaderStageCreateInfo.module = closestHitShader->getHandle();
				shaderStageCreateInfo.pName = "main";
				
				closestHitIndex = shadersStagesCreateInfos.size() - 1;
			}
			
			if (objectTypeInfo.anyHitShader)
			{
				VKPtr<VKShader>& anyHitShader = shaders.emplace_back(VKShader::create(_context, *objectTypeInfo.anyHitShader));
				
				vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo = shadersStagesCreateInfos.emplace_back();
				shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eAnyHitKHR;
				shaderStageCreateInfo.module = anyHitShader->getHandle();
				shaderStageCreateInfo.pName = "main";
				
				anyHitIndex = shadersStagesCreateInfos.size() - 1;
			}
			
			if (objectTypeInfo.intersectionShader)
			{
				VKPtr<VKShader>& intersectionShader = shaders.emplace_back(VKShader::create(_context, *objectTypeInfo.intersectionShader));
				
				vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo = shadersStagesCreateInfos.emplace_back();
				shaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eIntersectionKHR;
				shaderStageCreateInfo.module = intersectionShader->getHandle();
				shaderStageCreateInfo.pName = "main";
				
				intersectionIndex = shadersStagesCreateInfos.size() - 1;
			}
			
			vk::RayTracingShaderGroupCreateInfoKHR& shaderGroupCreateInfo = shadersGroupsCreateInfos.emplace_back();
			shaderGroupCreateInfo.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
			shaderGroupCreateInfo.generalShader = VK_SHADER_UNUSED_KHR;
			shaderGroupCreateInfo.closestHitShader = closestHitIndex != -1 ? closestHitIndex : VK_SHADER_UNUSED_KHR;
			shaderGroupCreateInfo.anyHitShader = anyHitIndex != -1 ? anyHitIndex : VK_SHADER_UNUSED_KHR;
			shaderGroupCreateInfo.intersectionShader = intersectionIndex != -1 ? intersectionIndex : VK_SHADER_UNUSED_KHR;
		}
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
	
	_context.getDevice().getRayTracingShaderGroupHandlesKHR(_pipeline, 0, 1, _raygenGroupHandle.size(), _raygenGroupHandle.data());
	
	size_t index = 1;
	_rayTypes.reserve(_info.getRayTypesInfos().size());
	for (const VKRayTracingPipelineInfo::RayTypeInfo& rayTypeInfo : _info.getRayTypesInfos())
	{
		RayType& rayType = _rayTypes.emplace_back();
		_context.getDevice().getRayTracingShaderGroupHandlesKHR(_pipeline, index, 1, rayType.missGroupHandle.size(), rayType.missGroupHandle.data());
		index++;
		
		rayType.objectsTypesHandles.reserve(rayTypeInfo.objectTypesInfos.size());
		for (int i = 0; i < _info.getRayTypesInfos().size(); i++)
		{
			std::array<std::byte, 32>& hitGroupHandle = rayType.objectsTypesHandles.emplace_back();
			_context.getDevice().getRayTracingShaderGroupHandlesKHR(_pipeline, index, 1, hitGroupHandle.size(), hitGroupHandle.data());
			index++;
		}
		
		_totalHitGroupCount += rayTypeInfo.objectTypesInfos.size();
	}
}

VKRayTracingPipeline::~VKRayTracingPipeline()
{
	_context.getDevice().destroyPipeline(_pipeline);
}

vk::PipelineBindPoint VKRayTracingPipeline::getPipelineType() const
{
	return vk::PipelineBindPoint::eRayTracingKHR;
}

const VKRayTracingPipelineInfo& VKRayTracingPipeline::getInfo() const
{
	return _info;
}

const VKPtr<VKPipelineLayout>& VKRayTracingPipeline::getPipelineLayout() const
{
	return _info.getPipelineLayout();
}

const std::array<std::byte, 32>& VKRayTracingPipeline::getRaygenGroupHandle() const
{
	return _raygenGroupHandle;
}

const std::array<std::byte, 32>& VKRayTracingPipeline::getMissGroupHandle(uint32_t index) const
{
	return _rayTypes[index].missGroupHandle;
}

size_t VKRayTracingPipeline::getMissGroupCount() const
{
	return _rayTypes.size();
}

const std::array<std::byte, 32>& VKRayTracingPipeline::getHitGroupHandle(uint32_t rayTypeIndex, uint32_t objectTypeIndex) const
{
	return _rayTypes[rayTypeIndex].objectsTypesHandles[objectTypeIndex];
}

size_t VKRayTracingPipeline::getHitGroupCount(uint32_t rayTypeIndex) const
{
	return _rayTypes[rayTypeIndex].objectsTypesHandles.size();
}

size_t VKRayTracingPipeline::getHitGroupCount() const
{
	return _totalHitGroupCount;
}