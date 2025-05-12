// Copyright Levi Spevakow (C) 2025

#include "IEventListener.h"
#include "EventDispatcher.h"

Tempus::IEventListener::IEventListener()
{
    EVENT_DISPATCHER->Subscribe(this);
}

Tempus::IEventListener::~IEventListener()
{
    EVENT_DISPATCHER->Unsubscribe(this);
}
