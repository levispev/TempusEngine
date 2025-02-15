#include "IEventListener.h"
#include "EventDispatcher.h"

Tempus::IEventListener::IEventListener()
{
    EventDispatcher::GetInstance()->Subscribe(std::bind(&IEventListener::OnEvent, this, std::placeholders::_1));
}
