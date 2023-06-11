#pragma once

#include "Cyph3D/VKObject/VKObject.h"
#include "Cyph3D/VKObject/Image/VKImageViewInfo.h"

class VKImageView : public VKObject
{
public:
	static VKPtr<VKImageView> create(VKContext& context, const VKImageViewInfo& info);
	
	~VKImageView() override;
	
	const VKImageViewInfo& getInfo() const;
	
	vk::ImageView& getHandle();
	
	uint32_t getFirstReferencedLayer() const;
	uint32_t getLastReferencedLayer() const;
	
	uint32_t getFirstReferencedLevel() const;
	uint32_t getLastReferencedLevel() const;

private:
	VKImageView(VKContext& context, const VKImageViewInfo& info);
	
	VKImageViewInfo _info;
	
	vk::ImageView _handle;
	
	glm::uvec2 _referencedLayerRange;
	glm::uvec2 _referencedLevelRange;
};