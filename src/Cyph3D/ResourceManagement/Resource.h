#pragma once

#include <string>
#include <memory>
#include <cassert>

template<typename TResource, typename... Args>
class Resource
{
public:
	explicit Resource(std::string name):
	_name(std::move(name))
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
	
	virtual void loadResourceImpl(Args... args) = 0;

private:
	void loadResource(Args... args)
	{
		loadResourceImpl(args...);
		_ready = true;
	}
	
	friend class ResourceManager;
};