#pragma once

#include "Events/Event.h"
#include "sdl/SDL.h"

namespace Tempus {

	class IEventListener
	{

	protected:
		IEventListener();
		virtual ~IEventListener() = default;

		virtual void OnEvent(const SDL_Event& event) = 0;

	};

}