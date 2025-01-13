// Copyright Levi Spevakow (C) 2025

#pragma once

#include "Tempus/Core.h"
#include "sdl/SDL.h"

namespace Tempus {

	class TEMPUS_API Window
	{
	public:

		Window();
		~Window();

		bool Init(const char* title,
			int x, int y, int w,
			int h, Uint32 flags);

		SDL_Window* GetNativeWindow() const;

	private:

		SDL_Window* m_Window = nullptr;

	};

}

