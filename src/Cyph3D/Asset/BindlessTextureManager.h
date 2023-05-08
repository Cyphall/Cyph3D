#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/VKObject/VKDynamic.h"

#include <vector>
#include <vulkan/vulkan.hpp>
#include <filesystem>
#include <stack>

class VKImageView;
class VKDescriptorSetLayout;
class VKDescriptorSet;

class BindlessTextureManager
{
public:
	BindlessTextureManager();
	
	uint32_t acquireIndex();
	void releaseIndex(uint32_t index);
	void setTexture(uint32_t index, const VKPtr<VKImageView>& texture, const VKPtr<VKSampler>& sampler);
	
	const VKPtr<VKDescriptorSetLayout>& getDescriptorSetLayout();
	const VKPtr<VKDescriptorSet>& getDescriptorSet();
	
	void onNewFrame();

private:
	struct TextureChange
	{
		uint32_t index;
		VKPtr<VKImageView> texture;
		VKPtr<VKSampler> sampler;
	};
	
	std::stack<uint32_t> _availableIndices;
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	VKDynamic<VKDescriptorSet> _descriptorSet;
	std::vector<std::vector<TextureChange>> _pendingChanges;
	
	uint32_t _upperBound;
	
	void expand();
};