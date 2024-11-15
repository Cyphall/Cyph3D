#pragma once

class VKContext;

class VKObject
{
public:
	virtual ~VKObject() = default;

	VKObject(const VKObject& other) = delete;
	VKObject& operator=(const VKObject& other) = delete;

	VKObject(VKObject&& other) = delete;
	VKObject& operator=(VKObject&& other) = delete;

protected:
	explicit VKObject(VKContext& context);

	VKContext& _context;
};