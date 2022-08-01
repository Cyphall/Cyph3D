#pragma once

#include <cassert>
#include <memory>
#include <string>

template<typename TResource, typename... Args>
class Resource
{
public:
	explicit Resource(const std::string& name):
	_name(name)
	{
	
	}
	
	virtual ~Resource() = default;
	
	Resource(const Resource& other) = delete;
	Resource& operator=(const Resource& other) = delete;
	
	Resource(Resource&& other) = delete;
	Resource& operator=(Resource&& other) = delete;
	
	bool isResourceReady() const
	{
		return _ready;
	}
	
	const TResource& getResource() const
	{
		assert(_ready);
		return *_resource;
	}
	
	const std::string& getName() const
	{
		return _name;
	}

protected:
	std::unique_ptr<TResource> _resource;
	const std::string _name;
	std::atomic_bool _ready = false;
	
	virtual void loadResourceImpl(Args... args)
	{}

private:
	void loadResource(Args... args)
	{
		loadResourceImpl(args...);
		_ready = true;
	}
	
	friend class ResourceManager;
};