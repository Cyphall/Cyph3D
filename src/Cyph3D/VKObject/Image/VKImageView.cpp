#include "VKImageView.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/VKHelper.h"

VKPtr<VKImageView> VKImageView::create(VKContext& context, const VKImageViewInfo& info)
{
	return VKPtr<VKImageView>(new VKImageView(context, info));
}

VKImageView::VKImageView(VKContext& context, const VKImageViewInfo& info):
	VKObject(context),
	_info(info)
{
	vk::ImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.image = _info.getImage()->getHandle();
	imageViewCreateInfo.viewType = _info.getViewType();
	
	if (_info.hasCustomViewFormat())
	{
		imageViewCreateInfo.format = _info.getCustomViewFormat();
	}
	else
	{
		imageViewCreateInfo.format = _info.getImage()->getInfo().getFormat();
	}
	
	if (_info.hasSwizzle())
	{
		imageViewCreateInfo.components.r = _info.getSwizzle()[0];
		imageViewCreateInfo.components.g = _info.getSwizzle()[1];
		imageViewCreateInfo.components.b = _info.getSwizzle()[2];
		imageViewCreateInfo.components.a = _info.getSwizzle()[3];
	}
	else
	{
		imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
	}
	
	imageViewCreateInfo.subresourceRange.aspectMask = VKHelper::getAspect(imageViewCreateInfo.format);
	
	if (_info.hasCustomLayerRange())
	{
		_referencedLayerRange = _info.getCustomLayerRange();
	}
	else
	{
		_referencedLayerRange = {0, _info.getImage()->getInfo().getLayers()-1};
	}
	
	imageViewCreateInfo.subresourceRange.baseArrayLayer = _referencedLayerRange.x;
	imageViewCreateInfo.subresourceRange.layerCount = _referencedLayerRange.y - _referencedLayerRange.x + 1;
	
	if (_info.hasCustomLevelRange())
	{
		_referencedLevelRange = _info.getCustomLevelRange();
	}
	else
	{
		_referencedLevelRange = {0, _info.getImage()->getInfo().getLevels()-1};
	}
	
	imageViewCreateInfo.subresourceRange.baseMipLevel = _referencedLevelRange.x;
	imageViewCreateInfo.subresourceRange.levelCount = _referencedLevelRange.y - _referencedLevelRange.x + 1;
	
	_handle = _context.getDevice().createImageView(imageViewCreateInfo);
}

VKImageView::~VKImageView()
{
	_context.getDevice().destroyImageView(_handle);
}

const VKImageViewInfo& VKImageView::getInfo() const
{
	return _info;
}

vk::ImageView& VKImageView::getHandle()
{
	return _handle;
}

uint32_t VKImageView::getFirstReferencedLayer() const
{
	return _referencedLayerRange.x;
}

uint32_t VKImageView::getLastReferencedLayer() const
{
	return _referencedLayerRange.y;
}

uint32_t VKImageView::getFirstReferencedLevel() const
{
	return _referencedLevelRange.x;
}

uint32_t VKImageView::getLastReferencedLevel() const
{
	return _referencedLevelRange.y;
}