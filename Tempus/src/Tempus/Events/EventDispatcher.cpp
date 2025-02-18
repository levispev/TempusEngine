// Copyright Levi Spevakow (C) 2025

#include "EventDispatcher.h"

Tempus::EventDispatcher* Tempus::EventDispatcher::s_Instance = new Tempus::EventDispatcher();

std::vector<std::function<void(const SDL_Event&)>> Tempus::EventDispatcher::subscribers;

void Tempus::EventDispatcher::Subscribe(std::function<void(const SDL_Event&)> callback)
{
	subscribers.push_back(callback);
}

void Tempus::EventDispatcher::Propagate(const SDL_Event& event)
{
	for (auto& callback : GetInstance()->subscribers) 
	{
		callback(event);
	}
}
