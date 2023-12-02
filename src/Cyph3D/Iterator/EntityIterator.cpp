#include "EntityIterator.h"

EntityIterator::EntityIterator(std::vector<Scene::EntityContainer>::iterator it):
	_it(it)
{}

EntityIterator& EntityIterator::operator++()
{
	_it++;
	return *this;
}

EntityIterator EntityIterator::operator++(int)
{
	EntityIterator temp = *this;
	++(*this);
	return temp;
}

bool EntityIterator::operator==(const EntityIterator& other) const
{
	return _it == other._it;
}

bool EntityIterator::operator!=(const EntityIterator& other) const
{
	return !this->operator==(other);
}

Entity& EntityIterator::operator*()
{
	return *(this->operator->());
}

Entity* EntityIterator::operator->()
{
	return _it->entity.get();
}

std::vector<Scene::EntityContainer>::iterator EntityIterator::getUnderlyingIterator()
{
	return _it;
}