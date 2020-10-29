#pragma once

#include <utility>
#include <string>
#include <memory>

template<typename TResource, typename TResourceLoadingData>
class Resource
{
public:
	explicit Resource(std::string name):
	_name(std::move(name))
	{
	
	}
	
	bool isResourceReady() const
	{
		return (bool)_resource;
	}
	
	TResource& getResource()
	{
		return *_resource;
	}
	
	const std::string& getName() const
	{
		return _name;
	}

protected:
	std::unique_ptr<TResource> _resource;
	std::string _name;
	
	virtual void finishLoading(const TResourceLoadingData& data) = 0;
};



