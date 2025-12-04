// Copyright Levi Spevakow (C) 2025

#pragma once

#include <memory>
#include <set>

#include "Event.h"
#include "SDL3/SDL.h"

#define EVENT_DISPATCHER EventDispatcher::GetInstance()

namespace Tempus {

	class IEventListener;

	typedef std::set<IEventListener*> SubscriberSet;
	
	class TEMPUS_API EventDispatcher {

	private:

		EventDispatcher() = default;

		static std::unique_ptr<EventDispatcher> s_Instance;
		static SubscriberSet subscribers;

	public:

		static EventDispatcher* GetInstance()
		{
			if (!s_Instance)
			{
				// Not using std::make_unique because of private constructor
				s_Instance = std::unique_ptr<EventDispatcher>(new EventDispatcher());
			}
			return s_Instance.get();
		}

		static const SubscriberSet& GetSubscribers() { return subscribers; }
		static uint32_t GetSubscriberCount() { return static_cast<uint32_t>(subscribers.size()); }
		static void Subscribe(IEventListener* subscriber);
		static void Unsubscribe(IEventListener* subscriber);
		static void Propagate(const SDL_Event& event);

	};
};

