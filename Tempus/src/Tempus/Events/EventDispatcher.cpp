// Copyright Levi Spevakow (C) 2025

#include "EventDispatcher.h"

#include "IEventListener.h"

std::unique_ptr<Tempus::EventDispatcher> Tempus::EventDispatcher::s_Instance = nullptr;

Tempus::SubscriberSet Tempus::EventDispatcher::subscribers;

void Tempus::EventDispatcher::Subscribe(IEventListener* subscriber)
{
	subscribers.emplace(subscriber);
}

void Tempus::EventDispatcher::Unsubscribe(IEventListener* subscriber)
{
	subscribers.erase(subscriber);
}

void Tempus::EventDispatcher::Propagate(const SDL_Event& event)
{
	for (IEventListener* subscriber : subscribers) 
	{
		if (subscriber)
		{
			subscriber->OnEvent(event);
		}
	}
}
