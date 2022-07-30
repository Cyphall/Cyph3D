#pragma once

#include <vector>

template <typename... Args>
class Event
{
public:
	using Subscriber = void(Args...);
	
	void operator+=(Subscriber subscriber);
	void operator-=(Subscriber subscriber);
	
	void invoke(Args... args);

private:
	std::vector<Subscriber*> _subscribers;
};

template <typename... Args>
void Event<Args...>::operator+=(Subscriber subscriber)
{
	_subscribers.push_back(subscriber);
}

template <typename... Args>
void Event<Args...>::operator-=(Subscriber subscriber)
{
	for (auto it = _subscribers.begin(); it != _subscribers.end(); it++)
	{
		if (*it == subscriber)
		{
			_subscribers.erase(it);
			return;
		}
	}
}

template <typename... Args>
void Event<Args...>::invoke(Args... args)
{
	for (auto& subscriber : _subscribers)
	{
		subscriber(args...);
	}
}