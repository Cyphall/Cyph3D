#include "ComponentConstIterator.h"

ComponentConstIterator::ComponentConstIterator(std::vector<Entity::ComponentContainer>::const_iterator it):
_it(it)
{}

ComponentConstIterator& ComponentConstIterator::operator++()
{
	_it++;
	return *this;
}

ComponentConstIterator ComponentConstIterator::operator++(int)
{
	ComponentConstIterator temp = *this;
	++(*this);
	return temp;
}

bool ComponentConstIterator::operator==(const ComponentConstIterator& other) const
{
	return _it == other._it;
}

bool ComponentConstIterator::operator!=(const ComponentConstIterator& other) const
{
	return !this->operator==(other);
}

const Component& ComponentConstIterator::operator*()
{
	return *(this->operator->());
}

const Component* ComponentConstIterator::operator->()
{
	return _it->component.get();
}

std::vector<Entity::ComponentContainer>::const_iterator ComponentConstIterator::getUnderlyingIterator()
{
	return _it;
}