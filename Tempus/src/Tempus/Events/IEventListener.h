// Copyright Levi Spevakow (C) 2025

#pragma once

#include <memory>

#include "Event.h"
#include "SDL3/SDL.h"

namespace Tempus {

	class TEMPUS_API IEventListener
	{
	protected:

		friend class EventDispatcher;

		IEventListener();
		virtual ~IEventListener();

		virtual void OnEvent(const SDL_Event& event) = 0;

	};

}