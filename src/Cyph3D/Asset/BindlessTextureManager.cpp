#include "BindlessTextureManager.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/VKContext.h"

BindlessTextureManager::BindlessTextureManager()
{
	_upperBound = 1000000;
	_upperBound -= 1024; // directional lights shadow maps
	_upperBound -= 1024; // point lights shadow maps

	_descriptorSets.resize(Engine::getVKContext().getConcurrentFrameCount());
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

void BindlessTextureManager::setTexture(uint32_t index, const VKPtr<VKImage>& texture, const VKPtr<VKSampler>& sampler)
{
	std::scoped_lock lock(_mutex);

	_descriptorSets[_currentFrame]->bindDescriptor(0, texture, sampler, index);

	for (int i = 0; i < Engine::getVKContext().getConcurrentFrameCount(); i++)
	{
		if (i == _currentFrame)
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

	return _descriptorSets[_currentFrame];
}

void BindlessTextureManager::onNewFrame()
{
	std::scoped_lock lock(_mutex);

	_currentFrame = (_currentFrame + 1) % Engine::getVKContext().getConcurrentFrameCount();

	for (const TextureChange& textureChange : _pendingChanges[_currentFrame])
	{
		_descriptorSets[_currentFrame]->bindDescriptor(0, textureChange.texture, textureChange.sampler, textureChange.index);
	}

	_pendingChanges[_currentFrame].clear();
}

void BindlessTextureManager::expand()
{
	uint32_t oldSize = _descriptorSets[_currentFrame] ? _descriptorSets[_currentFrame]->getInfo().getVariableSizeAllocatedCount() : 0;
	uint32_t newSize = oldSize > 0 ? oldSize * 2 : 16;

	VKDescriptorSetLayoutInfo descriptorSetLayoutInfo(false);
	descriptorSetLayoutInfo.addIndexedBinding(vk::DescriptorType::eCombinedImageSampler, _upperBound);
	VKPtr<VKDescriptorSetLayout> newDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), descriptorSetLayoutInfo);

	VKDescriptorSetInfo descriptorSetInfo(newDescriptorSetLayout);
	descriptorSetInfo.setVariableSizeAllocatedCount(newSize);

	std::vector<VKPtr<VKDescriptorSet>> newDescriptorSets(Engine::getVKContext().getConcurrentFrameCount());
	for (int i = 0; i < Engine::getVKContext().getConcurrentFrameCount(); i++)
	{
		newDescriptorSets[i] = VKDescriptorSet::create(Engine::getVKContext(), descriptorSetInfo);

		if (_descriptorSets[i] && _descriptorSetLayout)
		{
			_descriptorSets[i]->copyTo(0, 0, newDescriptorSets[i], 0, 0, oldSize);
		}
	}

	_descriptorSetLayout = newDescriptorSetLayout;
	_descriptorSets = std::move(newDescriptorSets);

	for (int64_t i = newSize - 1; i >= oldSize; i--)
	{
		_availableIndices.push(i);
	}
}