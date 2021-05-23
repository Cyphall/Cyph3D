#pragma once

#include "../Entity/Entity.h"
#include <memory>
#include <list>

class EntityIterator
{
public:
	using iterator_category = std::input_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = Entity;
	using pointer = value_type*;
	using reference = value_type&;
	
	EntityIterator();
	
	EntityIterator(std::list<std::unique_ptr<Entity>>::iterator it);
	
	EntityIterator& operator++();
	EntityIterator operator++(int);
	
	bool operator==(const EntityIterator& other) const;
	bool operator!=(const EntityIterator& other) const;
	
	Entity& operator*();
	Entity* operator->();
	
	std::list<std::unique_ptr<Entity>>::iterator getUnderlyingIterator();

private:
	std::list<std::unique_ptr<Entity>>::iterator _it;
};
