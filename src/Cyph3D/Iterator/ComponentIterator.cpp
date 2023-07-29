#include "ComponentIterator.h"

ComponentIterator::ComponentIterator(std::vector<Entity::ComponentContainer>::iterator it):
_it(it)
{}

ComponentIterator& ComponentIterator::operator++()
{
	_it++;
	return *this;
}

ComponentIterator ComponentIterator::operator++(int)
{
	ComponentIterator temp = *this;
	++(*this);
	return temp;
}

bool ComponentIterator::operator==(const ComponentIterator& other) const
{
	return _it == other._it;
}

bool ComponentIterator::operator!=(const ComponentIterator& other) const
{
	return !this->operator==(other);
}

Component& ComponentIterator::operator*()
{
	return *(this->operator->());
}

Component* ComponentIterator::operator->()
{
	return _it->component.get();
}

std::vector<Entity::ComponentContainer>::iterator ComponentIterator::getUnderlyingIterator()
{
	return _it;
}