#pragma once

#include "Cyph3D/Entity/Entity.h"

#include <vector>

class Component;

class ComponentIterator
{
public:
	using iterator_category = std::input_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = Component;
	using pointer = value_type*;
	using reference = value_type&;

	ComponentIterator() = default;

	explicit ComponentIterator(std::vector<Entity::ComponentContainer>::iterator it);

	ComponentIterator& operator++();
	ComponentIterator operator++(int);

	bool operator==(const ComponentIterator& other) const;
	bool operator!=(const ComponentIterator& other) const;

	Component& operator*();
	Component* operator->();

	std::vector<Entity::ComponentContainer>::iterator getUnderlyingIterator();

private:
	std::vector<Entity::ComponentContainer>::iterator _it;
};