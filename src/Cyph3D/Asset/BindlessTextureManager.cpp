#include "BindlessTextureManager.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"

BindlessTextureManager::BindlessTextureManager()
{
	_upperBound = std::min(
		Engine::getVKContext().getDescriptorIndexingProperties().maxPerStageDescriptorUpdateAfterBindSampledImages,
		Engine::getVKContext().getDescriptorIndexingProperties().maxPerStageDescriptorUpdateAfterBindSamplers);
	_upperBound -= 1024; // directional lights shadow maps
	_upperBound -= 1024; // point lights shadow maps
	
	_pendingChanges.resize(Engine::getVKContext().getConcurrentFrameCount());
	expand();
}

uint32_t BindlessTextureManager::acquireIndex()
{
	std::scoped_lock lock(_mutex);
	
	if (_availableIndices.empty())
	{
		expand();
	}
	
	uint32_t index = _availableIndices.top();
	_availableIndices.pop();
	
	return index;
}

void BindlessTextureManager::releaseIndex(uint32_t index)
{
	setTexture(index, {}, {});
	
	std::scoped_lock lock(_mutex);
	
	_availableIndices.push(index);
}

void BindlessTextureManager::setTexture(uint32_t index, const VKPtr<VKImageView>& texture, const VKPtr<VKSampler>& sampler)
{
	std::scoped_lock lock(_mutex);
	
	_descriptorSet->bindCombinedImageSampler(0, texture, sampler, index);
	
	for (int i = 0; i < Engine::getVKContext().getConcurrentFrameCount(); i++)
	{
		if (i == Engine::getVKContext().getCurrentConcurrentFrame())
		{
			continue;
		}
		
		TextureChange& textureChange = _pendingChanges[i].emplace_back();
		textureChange.index = index;
		textureChange.texture = texture;
		textureChange.sampler = sampler;
	}
}

const VKPtr<VKDescriptorSetLayout>& BindlessTextureManager::getDescriptorSetLayout()
{
	return _descriptorSetLayout;
}

const VKPtr<VKDescriptorSet>& BindlessTextureManager::getDescriptorSet()
{
	std::scoped_lock lock(_mutex);
	
	return _descriptorSet.getCurrent();
}

void BindlessTextureManager::onNewFrame()
{
	std::scoped_lock lock(_mutex);
	
	for (const TextureChange& textureChange : _pendingChanges[Engine::getVKContext().getCurrentConcurrentFrame()])
	{
		_descriptorSet->bindCombinedImageSampler(0, textureChange.texture, textureChange.sampler, textureChange.index);
	}
	
	_pendingChanges[Engine::getVKContext().getCurrentConcurrentFrame()].clear();
}

void BindlessTextureManager::expand()
{
	uint32_t oldSize = _descriptorSet ? _descriptorSet->getInfo().getVariableSizeAllocatedCount() : 0;
	uint32_t newSize = oldSize > 0 ? oldSize * 2 : 16;
	
	VKDescriptorSetLayoutInfo descriptorSetLayoutInfo(false);
	descriptorSetLayoutInfo.addIndexedBinding(vk::DescriptorType::eCombinedImageSampler, _upperBound);
	VKPtr<VKDescriptorSetLayout> newDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), descriptorSetLayoutInfo);
	
	VKDescriptorSetInfo descriptorSetInfo(newDescriptorSetLayout);
	descriptorSetInfo.setVariableSizeAllocatedCount(newSize);
	VKDynamic<VKDescriptorSet> newDescriptorSet = VKDynamic<VKDescriptorSet>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKDescriptorSet::create(context, descriptorSetInfo);
	});
	
	if (_descriptorSet && _descriptorSetLayout)
	{
		for (int i = 0; i < Engine::getVKContext().getConcurrentFrameCount(); i++)
		{
			_descriptorSet[i]->copyTo(0, 0, newDescriptorSet[i], 0, 0, oldSize);
		}
	}
	
	_descriptorSetLayout = newDescriptorSetLayout;
	_descriptorSet = std::move(newDescriptorSet);
	
	for (int64_t i = newSize - 1; i >= oldSize; i--)
	{
		_availableIndices.push(i);
	}
}