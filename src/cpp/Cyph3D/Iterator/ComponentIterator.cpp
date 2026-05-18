#include "ComponentIterator.h"

c3d::ComponentIterator::ComponentIterator(std::vector<Entity::ComponentContainer>::iterator it):
	_it(it)
{}

c3d::ComponentIterator& c3d::ComponentIterator::operator++()
{
	_it++;
	return *this;
}

c3d::ComponentIterator c3d::ComponentIterator::operator++(int)
{
	ComponentIterator temp = *this;
	++(*this);
	return temp;
}

bool c3d::ComponentIterator::operator==(const ComponentIterator& other) const
{
	return _it == other._it;
}

bool c3d::ComponentIterator::operator!=(const ComponentIterator& other) const
{
	return !this->operator==(other);
}

c3d::Component& c3d::ComponentIterator::operator*()
{
	return *(this->operator->());
}

c3d::Component* c3d::ComponentIterator::operator->()
{
	return _it->component.get();
}

std::vector<c3d::Entity::ComponentContainer>::iterator c3d::ComponentIterator::getUnderlyingIterator()
{
	return _it;
}