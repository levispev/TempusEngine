// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Events/Event.h"
#include "sdl/SDL.h"

#define BIND_EVENT(x) std::bind(&x, this, std::placeholders::_1)

namespace Tempus {

	class IEventListener
	{

	protected:

		IEventListener();
		virtual ~IEventListener() = default;

		virtual void OnEvent(const SDL_Event& event) = 0;

	};

}