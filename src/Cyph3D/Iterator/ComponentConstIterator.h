#pragma once

#include <memory>
#include <vector>

class Component;

class ComponentConstIterator
{
public:
	using iterator_category = std::input_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = Component;
	using pointer = value_type*;
	using reference = value_type&;
	
	ComponentConstIterator() = default;
	
	explicit ComponentConstIterator(std::vector<std::unique_ptr<Component>>::const_iterator it);
	
	ComponentConstIterator& operator++();
	ComponentConstIterator operator++(int);
	
	bool operator==(const ComponentConstIterator& other) const;
	bool operator!=(const ComponentConstIterator& other) const;
	
	const Component& operator*();
	const Component* operator->();
	
	std::vector<std::unique_ptr<Component>>::const_iterator getUnderlyingIterator();

private:
	std::vector<std::unique_ptr<Component>>::const_iterator _it;
};