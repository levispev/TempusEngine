// Copyright Levi Spevakow (C) 2025

#include "IEventListener.h"
#include "EventDispatcher.h"

Tempus::IEventListener::IEventListener()
{
    EventDispatcher::GetInstance()->Subscribe(BIND_EVENT(IEventListener::OnEvent));
}
