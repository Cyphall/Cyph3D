#include "EntityIterator.h"

EntityIterator::EntityIterator(std::list<std::unique_ptr<Entity>>::iterator it):
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
	return _it->get();
}

std::list<std::unique_ptr<Entity>>::iterator EntityIterator::getUnderlyingIterator()
{
	return _it;
}