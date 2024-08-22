#pragma once


#include <filesystem>
#include <mutex>
#include <stack>
#include <vector>

class VKImage;
class VKDescriptorSetLayout;
class VKDescriptorSet;
class VKSampler;

class BindlessTextureManager
{
public:
	BindlessTextureManager();

	uint32_t acquireIndex();
	void releaseIndex(uint32_t index);
	void setTexture(uint32_t index, const std::shared_ptr<VKImage>& texture, const std::shared_ptr<VKSampler>& sampler);

	const std::shared_ptr<VKDescriptorSetLayout>& getDescriptorSetLayout();
	const std::shared_ptr<VKDescriptorSet>& getDescriptorSet();

	void onNewFrame();

private:
	struct TextureChange
	{
		uint32_t index;
		std::shared_ptr<VKImage> texture;
		std::shared_ptr<VKSampler> sampler;
	};

	std::stack<uint32_t> _availableIndices;
	std::shared_ptr<VKDescriptorSetLayout> _descriptorSetLayout;
	std::vector<std::shared_ptr<VKDescriptorSet>> _descriptorSets;
	std::vector<std::vector<TextureChange>> _pendingChanges;

	uint32_t _currentFrame = 0;

	std::mutex _mutex;

	void expand();
};