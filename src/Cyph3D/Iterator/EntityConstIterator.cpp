#include "EntityConstIterator.h"

EntityConstIterator::EntityConstIterator(std::list<std::unique_ptr<Entity>>::const_iterator it):
_it(it)
{}

EntityConstIterator& EntityConstIterator::operator++()
{
	_it++;
	return *this;
}

EntityConstIterator EntityConstIterator::operator++(int)
{
	EntityConstIterator temp = *this;
	++(*this);
	return temp;
}

bool EntityConstIterator::operator==(const EntityConstIterator& other) const
{
	return _it == other._it;
}

bool EntityConstIterator::operator!=(const EntityConstIterator& other) const
{
	return !this->operator==(other);
}

const Entity& EntityConstIterator::operator*()
{
	return *(this->operator->());
}

const Entity* EntityConstIterator::operator->()
{
	return _it->get();
}

std::list<std::unique_ptr<Entity>>::const_iterator EntityConstIterator::getUnderlyingIterator()
{
	return _it;
}