// Copyright Levi Spevakow (C) 2025

#pragma once

#include <functional>
#include "Event.h"
#include "sdl/SDL.h"

#define EVENT_DISPATCHER EventDispatcher::GetInstance()

namespace Tempus {

	class EventDispatcher {

	private:

		EventDispatcher() = default;
		~EventDispatcher() = default;

		static EventDispatcher* s_Instance;
		static std::vector<std::function<void(const SDL_Event&)>> subscribers;

	public:

		static EventDispatcher* GetInstance() { return s_Instance; };
		static void Subscribe(std::function<void(const SDL_Event&)> callback);
		static void Propagate(const SDL_Event& event);

	};
};

