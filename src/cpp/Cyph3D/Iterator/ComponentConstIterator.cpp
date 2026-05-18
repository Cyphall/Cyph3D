#include "ComponentConstIterator.h"

c3d::ComponentConstIterator::ComponentConstIterator(std::vector<Entity::ComponentContainer>::const_iterator it):
	_it(it)
{}

c3d::ComponentConstIterator& c3d::ComponentConstIterator::operator++()
{
	_it++;
	return *this;
}

c3d::ComponentConstIterator c3d::ComponentConstIterator::operator++(int)
{
	ComponentConstIterator temp = *this;
	++(*this);
	return temp;
}

bool c3d::ComponentConstIterator::operator==(const ComponentConstIterator& other) const
{
	return _it == other._it;
}

bool c3d::ComponentConstIterator::operator!=(const ComponentConstIterator& other) const
{
	return !this->operator==(other);
}

const c3d::Component& c3d::ComponentConstIterator::operator*()
{
	return *(this->operator->());
}

const c3d::Component* c3d::ComponentConstIterator::operator->()
{
	return _it->component.get();
}

std::vector<c3d::Entity::ComponentContainer>::const_iterator c3d::ComponentConstIterator::getUnderlyingIterator()
{
	return _it;
}